#include <iostream>
#include <memory>
#include <string>
 
#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
 
#include "ssologin.grpc.pb.h"
 
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using ssologin::UserService;
using ssologin::User;
using ssologin::Credential;
 
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

