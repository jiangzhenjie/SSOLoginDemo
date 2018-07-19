#include <iostream>
#include <memory>
#include <string>
 
#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "third_party/mysql-helper/MySql.hpp"
#include "third_party/bcrypt/BCrypt.hpp"

#include "proto/ssologin.grpc.pb.h"
#include "common/ssologin_crypto.h"
 
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using ssologin::UserService;
using ssologin::User;
using ssologin::Credential;

#define SQL_HOST      "127.0.0.1"
#define SQL_USER      "root"
#define SQL_PASSWORD  "root"
#define SQL_TABLE     "ssologin"

const char *privateKey = "key/private.pem";
const std::string passwordSalt = "QNpoSjC49adVVEeXXuzSbJqDvsum7JTI";

class UserServiceImpl final : public UserService::Service {

  enum UserStatus { 
    Online = 0,
    Offline = 1
  };

  std::vector<std::string> invalidSessions; //invalid sessions
  bool invalidSessionsLock;

  std::string makeSeession() {

    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    char session[33];
    srand(time(0));
    for (int i = 0; i < 32; ++i) {
        session[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    session[32] = '\0';

    return std::string(session);
  }

  std::string decryptPassword(const std::string& password) {
    
    unsigned char base64[1024];
    size_t size;
      
    const char *input = password.c_str();
    base64_decode((unsigned char *)input, strlen(input), base64, &size, 1024);

    unsigned char plaintext[1024] = {0};
    size_t plaintext_len;

    int ret = rsa_private_decrypt(base64, size, privateKey, plaintext, &plaintext_len);
    if (ret == 0) {
      return std::string((char *)plaintext);
    }

    return "";
  }

  std::string hashPassword(const std::string& password) {
    BCrypt bcrypt;
    std::string hash = bcrypt.generateHash(password);
    return hash;
  }

  bool validatePassword(const std::string& password, const std::string& hash) {
    BCrypt bcrypt;
    return bcrypt.validatePassword(password, hash);
  }

  Status Register(ServerContext* context, const Credential* credential,
    User* user) override {
    std::cout << "[Notice]  Recive Register Request" << std::endl;

    if (!credential) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    if (credential->username().empty() || credential->password().empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "用户名或密码不能为空");
    }

    const std::string password = decryptPassword(credential->password());
    if (password.empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "用户名或密码不能为空");
    }

    try {

      MySql mysql(SQL_HOST, SQL_USER, SQL_PASSWORD, SQL_TABLE);

      std::vector<std::tuple<long, std::string>> users;
      mysql.runQuery(&users, "select uid, username from ssologin_user where username = ?", credential->username());

      if (users.size() > 0) {
        return Status(StatusCode::ALREADY_EXISTS, "用户已存在，请重新输入用户名");
      }

      std::string hashPwd = hashPassword(password);
      int ret = mysql.runCommand("insert into ssologin_user(username, password) values(?, ?)", credential->username(), hashPwd);
      if (ret <= 0) {
        return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
      }

      unsigned long long uid = mysql.getLastInsertID();
      std::string session = makeSeession();

      ret = mysql.runCommand("insert into ssologin_session(uid, session) values(?, ?)", uid, session);
      if (ret <= 0) {
        return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
      }

      user->set_status(UserStatus::Online);
      user->set_uid(std::to_string(uid));
      user->set_username(credential->username());
      user->set_session(session);

      return Status::OK;

    } catch (MySqlException e) {
      std::cout << e.what() << std::endl;
    }

    return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
  }

  Status Login(ServerContext* context, const Credential* credential,
                  User* user) override {
    std::cout << "[Notice] Recive Login Request" << std::endl;

    if (!credential) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    if (credential->username().empty() || credential->password().empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "用户名或密码不能为空");
    }

    const std::string password = decryptPassword(credential->password());
    if (password.empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "用户名或密码不能为空");
    }

    try {

      MySql mysql(SQL_HOST, SQL_USER, SQL_PASSWORD, SQL_TABLE);

      std::vector<std::tuple<long, std::string, std::string>> users;

      mysql.runQuery(&users, "select uid, username, password from ssologin_user where username = ?", credential->username());
      if (users.size() <= 0) {
        return Status(StatusCode::PERMISSION_DENIED, "用户不存在"); 
      }

      std::tuple<int, std::string, std::string> rowUser = users.front();
      std::string uid = std::to_string(std::get<0>(rowUser));
      std::string username = std::get<1>(rowUser);
      std::string hash = std::get<2>(rowUser);

      if (!validatePassword(password, hash)) {
        return Status(StatusCode::PERMISSION_DENIED, "用户名或密码错误"); 
      }

      // push to client for session invalid
      std::vector<std::tuple<std::string>> sessions;
      mysql.runQuery(&sessions, "select session from ssologin_session where uid = ?", uid);
      invalidSessionsLock = true;
      for (std::vector<std::tuple<std::string>>::iterator i = sessions.begin(); i != sessions.end(); ++i) {
        std::string session = std::get<0>(*i);
        invalidSessions.push_back(std::string(session));
      }
      invalidSessionsLock = false;

      // remove all online sessions
      mysql.runCommand("delete from ssologin_session where uid = ?", uid);

      // create new session
      std::string session = makeSeession();
      int ret = mysql.runCommand("insert into ssologin_session(uid, session) values(?, ?)", uid, session);
      if (ret <= 0) {
        return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
      }

      user->set_status(UserStatus::Online);
      user->set_uid(uid);
      user->set_username(username);
      user->set_session(session);

      std::cout << "[Notice] Login Succeed" << std::endl;

      return Status::OK;

    } catch (MySqlException e) {
      std::cout << e.what() << std::endl;
    }

    return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
  }

  Status Validate(ServerContext* context, const User* checkUser,
    User* respUser) override {
    std::cout << "[Notice] Recive Validate Request" << std::endl;

    if (checkUser->username().empty() || checkUser->session().empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "参数错误");
    }

    try {

      MySql mysql(SQL_HOST, SQL_USER, SQL_PASSWORD, SQL_TABLE);

      std::vector<std::tuple<long>> users;
      mysql.runQuery(&users, "select ssologin_user.uid from ssologin_user inner join ssologin_session on ssologin_user.uid = ssologin_session.uid where username = ? and session = ?",
                    checkUser->username(),
                    checkUser->session());

      if (users.size() <= 0) {
        respUser->set_status(UserStatus::Offline);
        respUser->set_username(checkUser->username());
      } else {
        std::string uid = std::to_string(std::get<0>(users.front()));
        respUser->set_status(UserStatus::Online);
        respUser->set_uid(uid);
        respUser->set_username(checkUser->username());
        respUser->set_session(checkUser->session());
      }

      return Status::OK;

    } catch (MySqlException e) {
      std::cout << e.what() << std::endl;
    }

    return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
  }

  Status Logout(ServerContext* context, const User* user,
    User* respUser) override {

    std::cout << "[Notice] Recive Logout Request" << std::endl;

    if (user->username().empty() || user->session().empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "参数错误");
    }

    try {

      MySql mysql(SQL_HOST, SQL_USER, SQL_PASSWORD, SQL_TABLE);

      mysql.runCommand("delete ssologin_session from ssologin_session inner join ssologin_user on ssologin_session.uid = ssologin_user.uid where username = ? and session = ?",
                      user->username(),
                      user->session());
      
      respUser->set_status(UserStatus::Offline);
      respUser->set_username(user->username());

      return Status::OK;

    } catch (MySqlException e) {
      std::cout << e.what() << std::endl;
    }

    return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
  }

  Status Notice(ServerContext* context, const User* user, ServerWriter<User>* writer) override {

    User response;
    Status status = Validate(context, user, &response);
    if (!status.ok()) {
      // 发生系统错误
      return status;
    }

    if (response.status() == UserStatus::Offline) {
      writer->Write(response);
      // 注册的User不在线，直接返回
      return Status::OK;
    }

    bool stop = false;
    while (!stop) {
      if (!invalidSessionsLock) {
        std::vector<std::string>::iterator i;
        for (i = invalidSessions.begin(); i != invalidSessions.end(); ++i) {
          if (*i == response.session()) {
            break;
          }
        }

        if (i != invalidSessions.end()) {
          invalidSessions.erase(i);
          stop = true;
        }
      }
    }

    writer->Write(response);

    return Status::OK;
  }

};
 
void RunServer() {
  std::string server_address("0.0.0.0:50051");
  UserServiceImpl service;
 
  ServerBuilder builder;
  // TODO: use secure credential
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
 
  server->Wait();
}
 
int main(int argc, char** argv) {
  RunServer();

  return 0;
}

