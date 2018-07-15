# SSOLoginDemo
基于 [gRPC](https://github.com/grpc) 框架的单点登录系统Demo

## 系统环境
**服务端：** gRPC C++    
**客户端：**    
	1. iOS（Objective-C，C++）    
	2. C++ 命令行（调试使用）   
**数据库：** Mysql

## 详细设计

### 数据及接口定义

基于单点登录功能，抽象出两个数据定义及四个接口定义，如下：

```protobuf
// 用户服务，包含注册、登录、验证、退出四个接口
service UserService {
	rpc Register (Credential) returns (User) {}
	rpc Login (Credential) returns (User) {}
	rpc Validate (User) returns (User) {}
	rpc Logout (User) returns (google.protobuf.Empty) {}
}

// 用户凭证
message Credential {
	string username = 1;
	string password = 2;
}

// 用户信息
message User {
	int32 status = 1;
	string uid = 2;
	string username = 3;
	string session = 4;
}
```

`UserService`：用户服务接口，包含单点登录功能用到的四个接口，分别是注册、登录、验证和退出。    
`Credential`：用户凭证，表示用户身份验证信息，包含帐号名和密码。（扩展：可扩展手机号和短信验证码字段，支持短信动态密码登录和注册）     
`User`：用户信息，表示用户身份信息，包含状态、用户ID、用户名、在线状态等字段。（扩展：可支持手机号、邮箱等更多用户信息字段）

### 数据库表设计

由上述数据定义及功能设计，抽象出数据库表设计。数据库包含两个表，分别是 `ssologin_user` 表和 `ssologin_session` 表。前者存储用户身份信息，包含用户名、密码等字段，后者存储用户登录状态信息，主要包含在线状态等。如下：

`ssologin_user` 表 

字段|字段名|类型|可空|是否主键|约束
---|---|---|---|---|---
用户ID|uid|int(11)|NO|YES|
用户名|username|varchar(100)|YES|NO|Unique
密码|password|varchar(1024)|YES|NO|
注册时间|regtime|timestamp|YES|YES|NO|

`ssologin_session` 表

字段|字段名|类型|可空|是否主键|约束
---|---|---|---|---|---
会话ID|sid|int(11)|NO|YES|
用户ID|uid|int(11)|NO|NO|
会话|session|varchar(1024)|YES|NO|
登录时间|ltime|timestamp|YES|NO|


## 使用指南

