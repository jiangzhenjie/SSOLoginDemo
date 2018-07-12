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
using ssologin::UserService;
using ssologin::User;
using ssologin::Credential;

class Database {

private:
  MYSQL *conn;
  std::string host;
  unsigned int port;
  std::string username;
  std::string password;

public:
  Database(std::string& host, unsigned int port, std::string& username, std::string& password) {
    this->host = host;
    this->port = port;
    this->username = username;
    this->password = password;
  }

  Database() {
    this->host = "127.0.0.1";
    this->port = 3306;
    this->username = "root";
    this->password = "root";
  }

  int connect() {
    conn = mysql_init(NULL);
    if (conn == NULL) {
      return 1;
    }

    if (mysql_real_connect(conn, host.c_str(), username.c_str(), password.c_str(), NULL, port, NULL, 0) == NULL) {
      return 1;
    }

    return 0;
  }

  void close() {
    mysql_close(conn);
  }

};

class UserServiceImpl final : public UserService::Service {
  Status Register(ServerContext* context, const Credential* credential,
    User* user) override {
    std::cout << "[Notice]  Recive Register Request" << std::endl;
    return Status::OK;
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

