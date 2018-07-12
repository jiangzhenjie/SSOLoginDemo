protoc --cpp_out=./ ssologin.proto
protoc --grpc_out=./ --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin ssologin.proto