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

};

class UserServiceImpl final : public UserService::Service {
  Status Register(ServerContext* context, const Credential* credential,
    User* user) override {
    std::cout << "[Notice]  Recive Register Request" << std::endl;

    if (credential->username().empty() || credential->password().empty()) {
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

    std::string insertSql("insert into ssologin_user(username, password) values('" + credential->username() + "','" + credential->password() + "')");
    std::cout << insertSql << std::endl;
    
    ret = db.query(insertSql, &num_rows, result);
    if (ret != 0) {
      return Status(StatusCode::INTERNAL, "系统繁忙，请稍后重试");
    }

    // TODO: make session

    db.close();

    return Status(StatusCode::OK, "注册成功");
  }

  Status Login(ServerContext* context, const Credential* credential,
                  User* user) override {
    std::cout << "[Notice] Recive Login Request" << std::endl;
    user->set_username("jiangzhenjie");
    user->set_session("qwertyuiop");
    return Status::OK;
  }

  Status Validate(ServerContext* context, const User* checkUser,
    User* respUser) override {
    std::cout << "[Notice] Recive Validate Request" << std::endl;

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

