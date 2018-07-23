#include <iostream>
#include <memory>
#include <string>
 
#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "third_party/bcrypt/BCrypt.hpp"

#include "proto/ssologin.grpc.pb.h"
#include "common/ssologin_crypto.h"
#include <SQLiteCpp/SQLiteCpp.h>
 
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

#define DB_FILE "database/ssologin.db3"

const char *privateKey = "key/private.pem";

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

      SQLite::Database db(DB_FILE, SQLite::OPEN_READWRITE);

      // query if username exists
      SQLite::Statement query(db, "select uid, username from ssologin_user where username = ?");
      query.bind(1, credential->username());
      if (query.executeStep()) {
        return Status(StatusCode::ALREADY_EXISTS, "用户已存在，请重新输入用户名");
      }

      // register user by insert username and password into ssologin_user
      std::string hashPwd = hashPassword(password);
      SQLite::Statement insert(db, "insert into ssologin_user(username, password) values(?, ?)");
      insert.bind(1, credential->username());
      insert.bind(2, hashPwd);
      int ret = insert.exec();
      if (ret <= 0) {
        return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
      }

      // login user by insert session into ssologin_session
      SQLite::Statement queryUid(db, "select uid from ssologin_user where username = ?");
      queryUid.bind(1, credential->username());
      if(!queryUid.executeStep()) {
        return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
      }
      long long uid = queryUid.getColumn(0);

      std::string session = makeSeession();
      SQLite::Statement insertSession(db, "insert into ssologin_session(uid, session) values(?, ?)");
      insertSession.bind(1, uid);
      insertSession.bind(2, session);
      ret = insertSession.exec();
      if (ret <= 0) {
        return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
      }

      user->set_status(UserStatus::Online);
      user->set_uid(std::to_string(uid));
      user->set_username(credential->username());
      user->set_session(session);

      return Status::OK;

    } catch (SQLite::Exception e) {
      std::cout << e.what() << std::endl;
      printf("Catch Exception [%s](%d)\n", __FUNCTION__, __LINE__);
    } catch (std::exception e) {
      std::cout << e.what() << std::endl;
      printf("Catch Exception [%s](%d)\n", __FUNCTION__, __LINE__);
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

      SQLite::Database db(DB_FILE, SQLite::OPEN_READWRITE);

      // query if username exists
      SQLite::Statement query(db, "select uid, username, password from ssologin_user where username = ?");
      query.bind(1, credential->username());
      if (!query.executeStep()) {
        return Status(StatusCode::ALREADY_EXISTS, "用户不存在");
      }
      long long uid = query.getColumn(0);
      std::string username = query.getColumn(1);
      std::string hash = query.getColumn(2);

      if (!validatePassword(password, hash)) {
        return Status(StatusCode::PERMISSION_DENIED, "用户名或密码错误"); 
      }

      // push to client for session invalid
      invalidSessionsLock = true;
      SQLite::Statement querySession(db, "select session from ssologin_session where uid = ?");
      querySession.bind(1, uid);
      while(querySession.executeStep()) {
        std::string session = querySession.getColumn(0);
        invalidSessions.push_back(std::string(session));
      }
      invalidSessionsLock = false;

      // remove all online sessions
      SQLite::Statement del(db, "delete from ssologin_session where uid = ?");
      del.bind(1, uid);
      del.exec();

      // // create new session
      std::string session = makeSeession();
      SQLite::Statement insert(db, "insert into ssologin_session(uid, session) values(?, ?)");
      insert.bind(1, uid);
      insert.bind(2, session);
      int ret = insert.exec();
      if (ret <= 0) {
        return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
      }

      user->set_status(UserStatus::Online);
      user->set_uid(std::to_string(uid));
      user->set_username(username);
      user->set_session(session);

      std::cout << "[Notice] Login Succeed" << std::endl;

      return Status::OK;

    } catch (SQLite::Exception e) {
      std::cout << e.what() << std::endl;
      printf("Catch Exception [%s](%d)\n", __FUNCTION__, __LINE__);
    } catch (std::exception e) {
      std::cout << e.what() << std::endl;
      printf("Catch Exception [%s](%d)\n", __FUNCTION__, __LINE__);
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

      SQLite::Database db(DB_FILE, SQLite::OPEN_READWRITE);

      SQLite::Statement query(db, "select ssologin_user.uid from ssologin_user inner join ssologin_session on ssologin_user.uid = ssologin_session.uid where username = ? and session = ?");
      query.bind(1, checkUser->username());
      query.bind(2, checkUser->session());

      if (query.executeStep()) {
        long long uid = query.getColumn(0);
        respUser->set_status(UserStatus::Online);
        respUser->set_uid(std::to_string(uid));
        respUser->set_username(checkUser->username());
        respUser->set_session(checkUser->session());
      } else {
        respUser->set_status(UserStatus::Offline);
        respUser->set_username(checkUser->username());
      }

      return Status::OK;

    } catch (SQLite::Exception e) {
      std::cout << e.what() << std::endl;
      printf("Catch Exception [%s](%d)\n", __FUNCTION__, __LINE__);
    } catch (std::exception e) {
      std::cout << e.what() << std::endl;
      printf("Catch Exception [%s](%d)\n", __FUNCTION__, __LINE__);
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

      SQLite::Database db(DB_FILE, SQLite::OPEN_READWRITE);

      SQLite::Statement del(db, "delete from ssologin_session where sid in (select sid from ssologin_session inner join ssologin_user on ssologin_session.uid = ssologin_user.uid where username = ? and session = ?)");
      del.bind(1, user->username());
      del.bind(2, user->session());
      del.exec();
      
      respUser->set_status(UserStatus::Offline);
      respUser->set_username(user->username());

      return Status::OK;

    } catch (SQLite::Exception e) {
      std::cout << e.what() << std::endl;
      printf("Catch Exception [%s](%d)\n", __FUNCTION__, __LINE__);
    } catch (std::exception e) {
      std::cout << e.what() << std::endl;
      printf("Catch Exception [%s](%d)\n", __FUNCTION__, __LINE__);
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

