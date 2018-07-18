# SSOLoginDemo
基于 [gRPC](https://github.com/grpc) 框架的单点登录系统Demo

## 系统环境
**服务端：** gRPC C++    
**客户端：**    
	1. iOS（Objective-C，C++）    
	2. C++ 命令行（调试使用）   
**数据库：** Mysql
**构建工具：** Bazel

## 设计概述
基于单点登录功能，提供注册、登录、验证、退出四个接口。数据库提供存储用户信息和用户会话状态两个主要功能。用户登录状态由会话（session）来维护，服务端可校验会话的有效状态。为实现单点登录功能，每个用户的会话始终只保持最新一份，其他会话转为失效状态。

## 详细设计

### 数据及接口定义

基于上述概要设计，抽象出两个数据定义及四个接口定义，如下：

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

### 接口设计

以下针对注册、登录、验证、退出四个接口的关键路径进行描述

#### 注册接口

<!--Client->Server: reg(username, password)
Server->Server: decrypt password
Server->Server: params check
Server->Database: query username
Database->Server: query response
Server->Server: check username exists
Server->Database: insert(username, hash-password)
Database->Server: success
Server->Server: make session
Server->Database: insert session
Database->Server: success
Server->Client: success(session)-->

![reg](https://github.com/jiangzhenjie/SSOLoginDemo/blob/master/document/image/reg.svg)

1. 注册时客户端提交用户名和RSA公钥加密密码；
2. 服务端针对加密密码进行解密，解密完成后校验用户名和密码的合法性；
3. 服务端通过用户名查询数据库；
4. 服务端判断是否从数据库中查到该用户名，如果查到，则表明用户名已存在；
5. 当用户名可用时，服务端计算密码的SHA256加盐密码，将用户名和密码摘要插入到数据库中；
6. 插入成功后，服务端生成一个会话Session；
7. 服务端将会话Session插到会话表中；
8. 插入成功后，服务端返回客户端登录状态。

#### 登录接口

<!--Client->Server: login(username, password)
Server->Server: decrypt password
Server->Server: params check
Server->Database: query(username, hash-password)
Database->Server: query response
Server->Server: check if query success
Server- ->Client: fail(username or password invalid)
Server->Database: delete all session
Database->Server: success
Server->Server: make session
Server->Database: insert session
Database->Server: success
Server->Client: success(session)-->

![login](https://github.com/jiangzhenjie/SSOLoginDemo/blob/master/document/image/login.svg)

1. 登录时客户端提交用户名和RSA公钥加密密码；
2. 服务端针对加密密码进行解密，解密完成后校验用户名和密码的合法性；
3. 服务端计算密码的SHA256加盐值，通过用户名和密码摘要查询数据库；
4. 服务端判断是否从数据库中查到记录，如果查不到，则表明用户名或密码存在错误；
5. 服务端从数据中删除当前用户的所有会话状态；
6. 服务端生成一个新的会话，并将新的会话插入到数据库中；
7. 服务端返回客户端登录状态。

#### 验证接口

<!--Client->Server: validate(username, session)
Server->Server: params check
Server->Database: query(username, session)
Database->Server: query response
Server->Server: check if query success
Server-- >Client: fail(session invalid)
Server->Client: success(session valid)-->

![validate](https://github.com/jiangzhenjie/SSOLoginDemo/blob/master/document/image/validate.svg)



#### 退出接口

<!--Client->Server: logout(username, session)
Server->Server: params check
Server->Database: delete(username, session)
Database->Server: delete response
Server->Client: logout success-->

![logout](https://github.com/jiangzhenjie/SSOLoginDemo/blob/master/document/image/logout.svg)

### 安全性考虑
1. 考虑到密码安全问题，登录、注册接口提交的密码都通过RSA公钥加密，服务端通过RSA私钥解密，防止传输过程被窃取；
2. 服务端在存储密码时，不存储明文密码，存储SHA256加盐（盐为随机生成的固定字符串）的密码摘要。

### 代码清单说明

## 使用指南

