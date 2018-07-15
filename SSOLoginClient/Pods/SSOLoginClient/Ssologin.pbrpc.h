#if !GPB_GRPC_FORWARD_DECLARE_MESSAGE_PROTO
#import "Ssologin.pbobjc.h"
#endif

#import <ProtoRPC/ProtoService.h>
#import <ProtoRPC/ProtoRPC.h>
#import <RxLibrary/GRXWriteable.h>
#import <RxLibrary/GRXWriter.h>

#if GPB_GRPC_FORWARD_DECLARE_MESSAGE_PROTO
  @class SSOCredential;
  @class SSOUser;
#else
#endif


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


@end

/**
 * Basic service implementation, over gRPC, that only does
 * marshalling and parsing.
 */
@interface SSOUserService : GRPCProtoService<SSOUserService>
- (instancetype)initWithHost:(NSString *)host NS_DESIGNATED_INITIALIZER;
+ (instancetype)serviceWithHost:(NSString *)host;
@end

NS_ASSUME_NONNULL_END
