#include <iostream>
#include <memory>
#include <string>
 
#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include <mysql.h>

#include "ssologin.grpc.pb.h"
#include "ssologin_crypto.h"
 
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using ssologin::UserService;
using ssologin::User;
using ssologin::Credential;

typedef char** DB_ROW;
class Database {

private:
  MYSQL *conn;
  std::string host;
  unsigned int port;
  std::string username;
  std::string password;
  std::string dbname;

public:
  Database(std::string& host, unsigned int port, std::string& username, std::string& password, std::string& dbname) {
    this->host = host;
    this->port = port;
    this->username = username;
    this->password = password;
    this->dbname = dbname;
  }

  Database() {
    this->host = "127.0.0.1";
    this->port = 3306;
    this->username = "root";
    this->password = "root";
    this->dbname = "ssologin";
  }

  int connect() {
    conn = mysql_init(NULL);
    if (conn == NULL) {
      return 1;
    }

    if (mysql_real_connect(conn, host.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, NULL, 0) == NULL) {
      return 1;
    }

    return 0;
  }

  void close() {
    mysql_close(conn);
  }

  int query(std::string& sql, unsigned long long* num_rows, std::vector<std::vector<char*>>& resultSet) {

    int ret = mysql_query(conn, sql.c_str());

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL) {
      *num_rows = mysql_affected_rows(conn);
      return ret;
    }

    *num_rows = mysql_num_rows(result);

    MYSQL_ROW row;
    unsigned int num_fields;
    unsigned int i;

    num_fields = mysql_num_fields(result);
    while ((row = mysql_fetch_row(result))) {

      std::vector<char *> vRow;
      for(i = 0; i < num_fields; i++) {
        vRow.push_back(row[i]);
      }
   
      resultSet.push_back(vRow);
    }

    mysql_free_result(result);

    return ret;
  }

  unsigned long long getInsertID() {
    return mysql_insert_id(conn);
  }

};

const char *privateKey = "key/private.pem";
const std::string passwordSalt = "QNpoSjC49adVVEeXXuzSbJqDvsum7JTI";

class UserServiceImpl final : public UserService::Service {

  enum UserStatus { 
    Online = 0,
    Offline = 1
  };

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
    
    unsigned char *base64;
    size_t size;
    base64_decode(password.c_str(), &base64, &size);

    unsigned char plaintext[1024] = {0};
    size_t plaintext_len;

    int ret = rsa_private_decrypt(base64, size, privateKey, plaintext, &plaintext_len);
    if (ret == 0) {
      return std::string((char *)plaintext);
    }

    return "";
  }

  std::string hashPassword(const std::string& password) {
    char output[65];
    std::string pwd = password + passwordSalt;
    sha256(pwd.c_str(), output);
    return std::string(output);
  }

  Status Register(ServerContext* context, const Credential* credential,
    User* user) override {
    std::cout << "[Notice]  Recive Register Request" << std::endl;

    const std::string password = decryptPassword(credential->password());

    if (credential->username().empty() || password.empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "用户名或密码不能为空");
    }

    Database db;
    if (db.connect() != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    std::string querySql("select * from ssologin_user where username = '" + credential->username() + "'");
    std::cout << querySql << std::endl;

    unsigned long long num_rows = 0;
    std::vector<std::vector<char*>> result;
    int ret = db.query(querySql, &num_rows, result);
    if (ret != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    if (num_rows > 0) {
      return Status(StatusCode::ALREADY_EXISTS, "用户已存在，请重新输入用户名");
    }

    std::string hashPwd = hashPassword(password);

    std::string insertSql("insert into ssologin_user(username, password) values('" + credential->username() + "','" + hashPwd + "')");
    std::cout << insertSql << std::endl;
    
    ret = db.query(insertSql, &num_rows, result);
    if (ret != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    unsigned long long uid = db.getInsertID();
    std::string session = makeSeession();
    insertSql = "insert into ssologin_session(uid, session) values('" + std::to_string(uid) + "','" + session + "')";
    ret = db.query(insertSql, &num_rows, result);
    if (ret != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    db.close();

    user->set_status(UserStatus::Online);
    user->set_uid(std::to_string(uid));
    user->set_username(credential->username());
    user->set_session(session);

    return Status(StatusCode::OK, "注册成功");
  }

  Status Login(ServerContext* context, const Credential* credential,
                  User* user) override {
    std::cout << "[Notice] Recive Login Request" << std::endl;
    
    const std::string password = decryptPassword(credential->password());

    if (credential->username().empty() || password.empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "用户名或密码不能为空");
    }

    Database db;
    if (db.connect() != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    std::string hashPwd = hashPassword(password);

    std::string sql("select * from ssologin_user where username = '" + credential->username() 
      + "' and password = '" + hashPwd + "'");
    unsigned long long num_rows = 0;
    std::vector<std::vector<char*>> result;

    int ret = db.query(sql, &num_rows, result);

    if (ret != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    if (num_rows <= 0) {
      return Status(StatusCode::PERMISSION_DENIED, "用户名或密码错误"); 
    }

    std::string uid, username;

    std::vector<char *> row = result.front();
    std::vector<char *>::iterator it = row.begin();

    uid = std::string(*it++);
    username = std::string(*it);

    // remove all online sessions
    sql = "delete from ssologin_session where uid = '" + uid + "'";
    ret = db.query(sql, &num_rows, result);
    if (ret != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    // create new session
    std::string session = makeSeession();
    sql = "insert into ssologin_session(uid, session) values('" + uid + "','" + session + "')";
    ret = db.query(sql, &num_rows, result);
    if (ret != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    db.close();

    user->set_status(UserStatus::Online);
    user->set_uid(uid);
    user->set_username(username);
    user->set_session(session);

    return Status::OK;
  }

  Status Validate(ServerContext* context, const User* checkUser,
    User* respUser) override {
    std::cout << "[Notice] Recive Validate Request" << std::endl;

    if (checkUser->username().empty() || checkUser->session().empty()) {
      return Status(StatusCode::INVALID_ARGUMENT, "参数错误");
    }

    Database db;
    if (db.connect() != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    std::string sql("select ssologin_user.uid from ssologin_user inner join ssologin_session on ssologin_user.uid = ssologin_session.uid where username = '" + checkUser->username() + "' and session = '" + checkUser->session() + "'");
    std::cout << sql << std::endl;

    unsigned long long num_rows = 0;
    std::vector<std::vector<char*>> result;

    int ret = db.query(sql, &num_rows, result);
    if (ret != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    if (num_rows <= 0) {
      respUser->set_status(UserStatus::Offline);
      respUser->set_username(checkUser->username());
    } else {

      std::string uid;

      std::vector<char *> row = result.front();
      std::vector<char *>::iterator it = row.begin();
      uid = std::string(*it);

      respUser->set_status(UserStatus::Online);
      respUser->set_uid(uid);
      respUser->set_username(checkUser->username());
      respUser->set_session(checkUser->session());
    }

    db.close();

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

