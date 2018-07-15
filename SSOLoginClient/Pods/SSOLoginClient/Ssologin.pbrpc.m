#if !defined(GPB_GRPC_PROTOCOL_ONLY) || !GPB_GRPC_PROTOCOL_ONLY
#import "Ssologin.pbrpc.h"
#import "Ssologin.pbobjc.h"
#import <ProtoRPC/ProtoRPC.h>
#import <RxLibrary/GRXWriter+Immediate.h>

#if defined(GPB_USE_PROTOBUF_FRAMEWORK_IMPORTS) && GPB_USE_PROTOBUF_FRAMEWORK_IMPORTS
#import <Protobuf/Empty.pbobjc.h>
#else
#import "google/protobuf/Empty.pbobjc.h"
#endif

@implementation SSOUserService

// Designated initializer
- (instancetype)initWithHost:(NSString *)host {
  self = [super initWithHost:host
                 packageName:@"ssologin"
                 serviceName:@"UserService"];
  return self;
}

// Override superclass initializer to disallow different package and service names.
- (instancetype)initWithHost:(NSString *)host
                 packageName:(NSString *)packageName
                 serviceName:(NSString *)serviceName {
  return [self initWithHost:host];
}

#pragma mark - Class Methods

+ (instancetype)serviceWithHost:(NSString *)host {
  return [[self alloc] initWithHost:host];
}

#pragma mark - Method Implementations

#pragma mark Register(Credential) returns (User)

- (void)registerWithRequest:(SSOCredential *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler{
  [[self RPCToRegisterWithRequest:request handler:handler] start];
}
// Returns a not-yet-started RPC object.
- (GRPCProtoCall *)RPCToRegisterWithRequest:(SSOCredential *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler{
  return [self RPCToMethod:@"Register"
            requestsWriter:[GRXWriter writerWithValue:request]
             responseClass:[SSOUser class]
        responsesWriteable:[GRXWriteable writeableWithSingleHandler:handler]];
}
#pragma mark Login(Credential) returns (User)

- (void)loginWithRequest:(SSOCredential *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler{
  [[self RPCToLoginWithRequest:request handler:handler] start];
}
// Returns a not-yet-started RPC object.
- (GRPCProtoCall *)RPCToLoginWithRequest:(SSOCredential *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler{
  return [self RPCToMethod:@"Login"
            requestsWriter:[GRXWriter writerWithValue:request]
             responseClass:[SSOUser class]
        responsesWriteable:[GRXWriteable writeableWithSingleHandler:handler]];
}
#pragma mark Validate(User) returns (User)

- (void)validateWithRequest:(SSOUser *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler{
  [[self RPCToValidateWithRequest:request handler:handler] start];
}
// Returns a not-yet-started RPC object.
- (GRPCProtoCall *)RPCToValidateWithRequest:(SSOUser *)request handler:(void(^)(SSOUser *_Nullable response, NSError *_Nullable error))handler{
  return [self RPCToMethod:@"Validate"
            requestsWriter:[GRXWriter writerWithValue:request]
             responseClass:[SSOUser class]
        responsesWriteable:[GRXWriteable writeableWithSingleHandler:handler]];
}
#pragma mark Logout(User) returns (Empty)

- (void)logoutWithRequest:(SSOUser *)request handler:(void(^)(GPBEmpty *_Nullable response, NSError *_Nullable error))handler{
  [[self RPCToLogoutWithRequest:request handler:handler] start];
}
// Returns a not-yet-started RPC object.
- (GRPCProtoCall *)RPCToLogoutWithRequest:(SSOUser *)request handler:(void(^)(GPBEmpty *_Nullable response, NSError *_Nullable error))handler{
  return [self RPCToMethod:@"Logout"
            requestsWriter:[GRXWriter writerWithValue:request]
             responseClass:[GPBEmpty class]
        responsesWriteable:[GRXWriteable writeableWithSingleHandler:handler]];
}
@end
#endif
