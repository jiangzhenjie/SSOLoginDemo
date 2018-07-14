/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "ssologin.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using ssologin::UserService;
using ssologin::User;
using ssologin::Credential;

class UserClient {
 public:
  UserClient(std::shared_ptr<Channel> channel)
      : stub_(UserService::NewStub(channel)) {}

  // login interface for client-side
  Status Login(const std::string& username, const std::string& password, User* user) {
    
    Credential credential;
    credential.set_username(username);
    // TODO: password should be encrypted
    credential.set_password(password);

    ClientContext context;

    Status status = stub_->Login(&context, credential, user);

    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
    }

    return status;
  }

  // register interface for client-side
  Status Register(const std::string& username, const std::string& password, User* user) {

    Credential credential;
    credential.set_username(username);
    // TODO: password should be encrypted
    credential.set_password(password);

    ClientContext context;

    Status status = stub_->Register(&context, credential, user);

    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
    }

    return status;
  }

  Status Validate(const User& checkUser, User* respUser) {

    ClientContext context;
    Status status = stub_->Validate(&context, checkUser, respUser);

    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
    }

    return status;
  }
    
 private:
  std::unique_ptr<UserService::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  UserClient userclient(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  // Login Example
  // std::string username("jzj");
  // std::string password("123qwe");
  // User user;
  // Status status = userclient.Login(username, password, &user);
  // if (status.ok()) {
  //   std::cout << "Login Succeed, Welcome " + user.username() << std::endl;
  // }

  // Register Example
  std::string username("jiangzhenjie3");
  std::string password("1111");
  User user;
  Status status = userclient.Register(username, password, &user);
  if (status.ok()) {
    std::cout << "Register Succeed, Welcome " + user.username() << std::endl;
  }

  // Validate Example
  // User respUser;
  // Status status = userclient.Validate(user, &respUser);
  // if (status.ok()) {
  //   std::cout << "Validate Succeed, Welcome " + respUser.username() << std::endl;
  // }
    
  return 0;
}
