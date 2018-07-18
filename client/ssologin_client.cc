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

#include "proto/ssologin.grpc.pb.h"
#include "common/ssologin_crypto.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using ssologin::UserService;
using ssologin::User;
using ssologin::Credential;

const char *publicKey = "key/public.pem";

class UserClient {
 public:
  UserClient(std::shared_ptr<Channel> channel)
      : stub_(UserService::NewStub(channel)) {}

  std::string encryptPassword(const std::string& password) {

    const char *pwd = password.c_str();

    unsigned char ciphertext[1024] = {0};
    size_t ciphertext_len;

    int ret = rsa_public_encrypt((unsigned char *)pwd, strlen(pwd), publicKey, ciphertext, &ciphertext_len);
    if (ret == 0) {
      unsigned char base64[1024] = {0};
      base64_encode(ciphertext, ciphertext_len, base64);
      return std::string((const char *)base64);
    }
    return "";
  }

  // login interface for client-side
  Status Login(const std::string& username, const std::string& password, User* user) {
    
    Credential credential;
    credential.set_username(username);
    credential.set_password(encryptPassword(password));

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
    credential.set_password(encryptPassword(password));

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

  Status Logout(const User& user) {

    User response;

    ClientContext context;
    Status status = stub_->Logout(&context, user, &response);

    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
    }

    return status;
  }

  Status Notice(char **argv) {

    ClientContext context;

    User request;
    request.set_username(std::string(argv[1]));
    request.set_session(std::string(argv[2]));

    User response;

    std::unique_ptr<ClientReader<User> > reader(stub_->Notice(&context, request));
    while(reader->Read(&response)) {
      std::cout << response.username() << std::endl;
    }
    Status status = reader->Finish();
    if (status.ok()) {
      std::cout << "Notice Received Succeeded." << std::endl;
    } else {
      std::cout << "Notice Received Failed." << std::endl;
    }

    return Status::OK;
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
  std::string username("jiangzhenjie");
  std::string password("123qwe");
  User user;
  Status status = userclient.Login(username, password, &user);
  if (status.ok()) {
    std::cout << "Login Succeed, Welcome " + user.username() << std::endl;
  }

  // Register Example
  // std::string username("jiangzhenjie");
  // std::string password("123qwe");
  // User user;
  // Status status = userclient.Register(username, password, &user);
  // if (status.ok()) {
  //   std::cout << "Register Succeed, Welcome " + user.username() << std::endl;
  //   std::cout << user.session() << std::endl;
  // }

  //Validate Example
  // User checkUser;
  // checkUser.set_username("jiangzhenjie");
  // checkUser.set_session("uzLWKJZmmfvITErwXulhYBmUDDaKzZfw");
  // User respUser;
  // Status status = userclient.Validate(checkUser, &respUser);
  // if (status.ok()) {
  //   if (respUser.status() == 0) {
  //     std::cout << "Validate Succeed, Welcome " + respUser.username() << std::endl;
  //   } else {
  //     std::cout << "Validate Failed" << std::endl;
  //   }
  // }

  //Logout Example
  // User user;
  // user.set_username("jiangzhenjie3");
  // user.set_session("111111");
  // Status status = userclient.Logout(user);
  // if (status.ok()) {
  //   std::cout << "Logout Succeed" << std::endl;
  // }

  //Notice Example
  // if (argc != 3) {
  //     printf("%s\n", "please input session. eg. ./user_client [username] [session]");
  //     exit(0);
  // }
  // userclient.Notice(argv);
  
  return 0;
}
