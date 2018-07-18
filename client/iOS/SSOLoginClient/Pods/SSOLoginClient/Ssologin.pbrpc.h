#if !defined(GPB_GRPC_FORWARD_DECLARE_MESSAGE_PROTO) || !GPB_GRPC_FORWARD_DECLARE_MESSAGE_PROTO
#import "Ssologin.pbobjc.h"
#endif

#if !defined(GPB_GRPC_PROTOCOL_ONLY) || !GPB_GRPC_PROTOCOL_ONLY
#import <ProtoRPC/ProtoService.h>
#import <ProtoRPC/ProtoRPC.h>
#import <RxLibrary/GRXWriteable.h>
#import <RxLibrary/GRXWriter.h>
#endif

@class GPBEmpty;
@class SSOCredential;
@class SSOUser;

#if !defined(GPB_GRPC_FORWARD_DECLARE_MESSAGE_PROTO) || !GPB_GRPC_FORWARD_DECLARE_MESSAGE_PROTO
#if defined(GPB_USE_PROTOBUF_FRAMEWORK_IMPORTS) && GPB_USE_PROTOBUF_FRAMEWORK_IMPORTS
  #import <Protobuf/Empty.pbobjc.h>
#else
  #import "google/protobuf/Empty.pbobjc.h"
#endif
#endif

@class GRPCProtoCall;


NS_ASSUME_NONNULL_BEGIN

@protocol SSOUserService <NSObject>

#pragma mark Register(Credential) returns (User)

- (void)registerWithRequest:(SSOCredential *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler;

- (GRPCProtoCall *)RPCToRegisterWithRequest:(SSOCredential *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler;


#pragma mark Login(Credential) returns (User)

- (void)loginWithRequest:(SSOCredential *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler;

- (GRPCProtoCall *)RPCToLoginWithRequest:(SSOCredential *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler;


#pragma mark Validate(User) returns (User)

- (void)validateWithRequest:(SSOUser *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler;

- (GRPCProtoCall *)RPCToValidateWithRequest:(SSOUser *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler;


#pragma mark Logout(User) returns (Empty)

- (void)logoutWithRequest:(SSOUser *)request handler:(void(^)(GPBEmpty *_Nullable response, NSError *_Nullable error))handler;

- (GRPCProtoCall *)RPCToLogoutWithRequest:(SSOUser *)request handler:(void(^)(GPBEmpty *_Nullable response, NSError *_Nullable error))handler;


@end


#if !defined(GPB_GRPC_PROTOCOL_ONLY) || !GPB_GRPC_PROTOCOL_ONLY
/**
 * Basic service implementation, over gRPC, that only does
 * marshalling and parsing.
 */
@interface SSOUserService : GRPCProtoService<SSOUserService>
- (instancetype)initWithHost:(NSString *)host NS_DESIGNATED_INITIALIZER;
+ (instancetype)serviceWithHost:(NSString *)host;
@end
#endif

NS_ASSUME_NONNULL_END

