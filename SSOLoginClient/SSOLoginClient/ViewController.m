//
//  ViewController.m
//  SSOLoginClient
//
//  Created by jiangzhenjie on 2018/7/15.
//  Copyright © 2018年 JiangZhenjie. All rights reserved.
//

#import "ViewController.h"
#import <GRPCClient/GRPCCall+ChannelArg.h>
#import <GRPCClient/GRPCCall+Tests.h>
#import <SSOLoginClient/Ssologin.pbrpc.h>

static NSString * const kHostAddress = @"192.168.99.119:50051";

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    [GRPCCall useInsecureConnectionsForHost:kHostAddress];
    [GRPCCall setUserAgentPrefix:@"SSOLoginDemo/1.0" forHost:kHostAddress];
    
    SSOUserService *userService = [[SSOUserService alloc] initWithHost:kHostAddress];
    
    SSOCredential *credential = [SSOCredential message];
    credential.username = @"jiangzhenjie";
    credential.password = @"123qwe";
    
    [userService loginWithRequest:credential handler:^(SSOUser * _Nullable response, NSError * _Nullable error) {
        NSLog(@"<(Line: %d) %s> PPLog %@", __LINE__, __func__, response);
        NSLog(@"<(Line: %d) %s> PPLog %@", __LINE__, __func__, error);
    }];
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
