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

  Status Notice(const User& user) {

    ClientContext context;
    User response;

    std::unique_ptr<ClientReader<User> > reader(stub_->Notice(&context, user));
    while(reader->Read(&response)) {
    }
    Status status = reader->Finish();
    if (status.ok()) {
      std::cout << "Receivce Logout Notice, " + response.username() + " had logout" << std::endl;
    } else {
      std::cout << "Notice Received Failed." << std::endl;
    }

    return Status::OK;
  }
    
 private:
  std::unique_ptr<UserService::Stub> stub_;
};

UserClient createUserClient() {
  UserClient userclient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  return userclient;
}

void testLogin(char** argv) {
   UserClient userclient = createUserClient();

   std::string username(argv[2]);
   std::string password(argv[3]);
   User user;
   Status status = userclient.Login(username, password, &user);
   if (status.ok()) {
     std::cout << "Login Succeed, Welcome " + user.username() << std::endl;
   }
}

void testReg(char** argv) {
  UserClient userclient = createUserClient();

  std::string username(argv[2]);
  std::string password(argv[3]);
  User user;
  Status status = userclient.Register(username, password, &user);
  if (status.ok()) {
   std::cout << "Register Succeed, Welcome " + user.username() << std::endl;
  }
}

void testValidate(char** argv) {
  UserClient userclient = createUserClient();

  User checkUser;
  checkUser.set_username(std::string(argv[2]));
  checkUser.set_session(std::string(argv[3]));
  User respUser;
  Status status = userclient.Validate(checkUser, &respUser);
  if (status.ok()) {
    if (respUser.status() == 0) {
      std::cout << "Validate Succeed, Welcome " + respUser.username() << std::endl;
    } else {
      std::cout << "Validate Failed" << std::endl;
    }
  }
}

void testLogout(char** argv) {
  UserClient userclient = createUserClient();

  User user;
  user.set_username(std::string(argv[2]));
  user.set_session(std::string(argv[3]));
  Status status = userclient.Logout(user);
  if (status.ok()) {
    std::cout << user.username() + " Logout Succeed" << std::endl;
  }
}

void testNotice(char** argv) {
  UserClient userclient = createUserClient();

  User user;
  user.set_username(std::string(argv[2]));
  user.set_session(std::string(argv[3]));
  userclient.Notice(user);
}

int main(int argc, char** argv) {

  if (argc < 4) {
    printf("\n\nUsage: user_client <command> <options>");
    printf("\n\nAvailable commands:\nlogin\nreg\nvalidate\nlogout\nnotice");
    printf("\n\nOptions for command:\nlogin: username password\nreg: username password\nvalidate: username session\nlogout: username, session\nnotice: username, session");
    printf("\n\nExample:\nuser_client login jiangzhenjie 123qwe\nuser_client validate jiangzhenjie zNAloUKJreOMpquTxktIhAmKLAvPjGCw\n\n");
    exit(0);
  }

  char *command = argv[1];
  if (strcmp(command, "login") == 0) {
    testLogin(argv);
  } else if (strcmp(command, "reg") == 0) {
    testReg(argv);
  } else if (strcmp(command, "validate") == 0) {
    testValidate(argv);
  } else if (strcmp(command, "logout") == 0) {
    testLogout(argv);
  } else if (strcmp(command, "notice") == 0) {
    testNotice(argv);
  } else {
    printf("\n\n%s\n\n", "Unknow command");
  }
  
  return 0;
}
