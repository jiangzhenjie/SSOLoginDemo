//
//  UserViewController.m
//  SSOLoginClient
//
//  Created by jiangzhenjie on 2018/7/15.
//  Copyright © 2018年 JiangZhenjie. All rights reserved.
//

#import "UserViewController.h"
#import <SSOLoginClient/Ssologin.pbrpc.h>

@interface UserViewController ()

@property (weak, nonatomic) IBOutlet UILabel *welcomeLabel;

@end

@implementation UserViewController

- (instancetype)initWithUser:(SSOUser *)user {
    self = [super init];
    if (self) {
        _user = user;
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.welcomeLabel.text = [@"欢迎，" stringByAppendingString:self.user.username];
}

- (IBAction)pressLogoutBtn:(UIButton *)sender {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

@end
