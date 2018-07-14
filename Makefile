subdir=./
 
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
 
SOURCES = $(wildcard $(subdir)*.cc)
SRCOBJS = $(patsubst %.cc,%.o,$(SOURCES))
CC = g++

%.o:%.cc
	$(CC) -std=c++11 -I/usr/local/include -I/usr/local/mysql/include -pthread -c $< -o $@
 
all: user_client user_server
 
user_client:	ssologin.grpc.pb.o ssologin.pb.o ssologin_client.o ssologin_crypto.o
	$(CC) $^ -L/usr/local/lib -L/usr/local/mysql/lib `pkg-config --libs protobuf grpc++ grpc` -lgrpc++_reflection -ldl -lcrypto -lssl -lmysqlclient -o $@
 
user_server:	ssologin.grpc.pb.o ssologin.pb.o ssologin_server.o ssologin_crypto.o
	$(CC) $^ -L/usr/local/lib -L/usr/local/mysql/lib `pkg-config --libs protobuf grpc++ grpc` -lgrpc++_reflection -ldl -lcrypto -lssl -lmysqlclient -o $@
 
clean:
	rm -f *.o *.pb.cc *.pb.h user_client user_server
