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

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string Login(const std::string& username) {
    // Data we are sending to the server.
    Credential credential;
    credential.set_username(username);

    // Container for the data we expect from the server.
    User user;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Login(&context, credential, &user);

    // Act upon its status.
    if (status.ok()) {
      return "username: " + user.username() + " session: " + user.session();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
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
  std::string username("jzj");
  std::string reply = userclient.Login(username);
  std::cout << "UserClient received: " << reply << std::endl;
    
  return 0;
}
