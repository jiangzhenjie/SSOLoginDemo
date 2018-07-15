#import "Ssologin.pbrpc.h"
#import "Ssologin.pbobjc.h"

#import <ProtoRPC/ProtoRPC.h>
#import <RxLibrary/GRXWriter+Immediate.h>

@implementation SSOUserService

// Designated initializer
- (instancetype)initWithHost:(NSString *)host {
  return (self = [super initWithHost:host packageName:@"ssologin" serviceName:@"UserService"]);
}

// Override superclass initializer to disallow different package and service names.
- (instancetype)initWithHost:(NSString *)host
                 packageName:(NSString *)packageName
                 serviceName:(NSString *)serviceName {
  return [self initWithHost:host];
}

+ (instancetype)serviceWithHost:(NSString *)host {
  return [[self alloc] initWithHost:host];
}


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
@end
