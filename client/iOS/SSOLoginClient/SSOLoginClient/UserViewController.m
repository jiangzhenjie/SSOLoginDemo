//
//  UserViewController.m
//  SSOLoginClient
//
//  Created by jiangzhenjie on 2018/7/15.
//  Copyright © 2018年 JiangZhenjie. All rights reserved.
//

#import "UserViewController.h"
#import <SSOLoginClient/Ssologin.pbrpc.h>
#import "SSODefines.h"

@interface UserViewController ()

@property (weak, nonatomic) IBOutlet UILabel *welcomeLabel;
@property (weak, nonatomic) IBOutlet UILabel *errorLabel;

@property (nonatomic, strong) UIActivityIndicatorView *indicatorView;

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
    [self registerUserNotice];
}

- (void)registerUserNotice {
    SSOUserService *userService = [[SSOUserService alloc] initWithHost:kHostAddress];
    [userService noticeWithRequest:self.user eventHandler:^(BOOL done, SSOUser * _Nullable response, NSError * _Nullable error) {
        if (error == nil) {
            UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil message:@"你的帐号在别的设备登录成功，当前设备将下线" preferredStyle:UIAlertControllerStyleAlert];
            [alert addAction:[UIAlertAction actionWithTitle:@"确认" style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                [self clearUser];
                [self dismissViewControllerAnimated:YES completion:nil];
            }]];
            [self presentViewController:alert animated:YES completion:nil];
        }
    }];
}

- (void)showLoading {
    if (self.indicatorView == nil) {
        self.indicatorView = [[UIActivityIndicatorView alloc] init];
        self.indicatorView.activityIndicatorViewStyle = UIActivityIndicatorViewStyleGray;
        self.indicatorView.center = self.view.center;
        [self.view addSubview:self.indicatorView];
    }
    [self.indicatorView startAnimating];
    self.indicatorView.hidden = NO;
}

- (void)hideLoading {
    [self.indicatorView stopAnimating];
    self.indicatorView.hidden = YES;
}

- (IBAction)pressLogoutBtn:(UIButton *)sender {
    [self showLoading];
    SSOUserService *userService = [[SSOUserService alloc] initWithHost:kHostAddress];
    [userService logoutWithRequest:self.user handler:^(SSOUser * _Nullable response, NSError * _Nullable error) {
        if (error == nil) {
            [self hideLoading];
            [self clearUser];
            [self dismissViewControllerAnimated:YES completion:nil];
        } else {
            self.errorLabel.text = error.localizedDescription;
            self.errorLabel.hidden = NO;
        }
    }];
}

- (void)clearUser {
    [[NSUserDefaults standardUserDefaults] setObject:nil forKey:kUserDefaultLoginUser];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

@end
