subdir=./
 
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
 
SOURCES = $(wildcard $(subdir)*.cc)
SRCOBJS = $(patsubst %.cc,%.o,$(SOURCES))
CC = g++

%.o:%.cc
	$(CC) -std=c++11 -I/usr/local/include -pthread -c $< -o $@
 
all: user_client user_server
 
user_client:	ssologin.grpc.pb.o ssologin.pb.o ssologin_client.o
	$(CC) $^ -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -lgrpc++_reflection -ldl -o $@
 
user_server:	ssologin.grpc.pb.o ssologin.pb.o ssologin_server.o
	$(CC) $^ -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -lgrpc++_reflection -ldl -o $@
 
clean:
	rm -f *.o *.pb.cc *.pb.h user_client user_server
