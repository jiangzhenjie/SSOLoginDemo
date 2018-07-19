//
//  ViewController.m
//  SSOLoginClient
//
//  Created by jiangzhenjie on 2018/7/15.
//  Copyright © 2018年 JiangZhenjie. All rights reserved.
//

#import "ViewController.h"
#import <SSOLoginClient/Ssologin.pbrpc.h>
#import "SSODefines.h"
#import "RegLoginViewController.h"
#import "UserViewController.h"

@interface ViewController () <RegLoginViewControllerDelegate>

@property (weak, nonatomic) IBOutlet UIButton *regBtn;
@property (weak, nonatomic) IBOutlet UIButton *loginBtn;

@property (nonatomic, strong) UIActivityIndicatorView *indicatorView;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self validateUser];
}

- (void)validateUser {
    NSData *data = [[NSUserDefaults standardUserDefaults] objectForKey:kUserDefaultLoginUser];
    SSOUser *user = [[SSOUser alloc] initWithData:data error:NULL];
    if (user) {
        
        SSOUserService *userService = [[SSOUserService alloc] initWithHost:kHostAddress];
        [userService validateWithRequest:user handler:^(SSOUser * _Nullable response, NSError * _Nullable error) {
            if (error == nil && response.status == 0) {
                [self saveUser:response];
                UserViewController *userViewController = [self.storyboard instantiateViewControllerWithIdentifier:@"UserViewController"];
                userViewController.user = user;
                [self presentViewController:userViewController animated:YES completion:nil];
            }
        }];
    }
}

- (void)hideButtons {
    self.regBtn.hidden = YES;
    self.loginBtn.hidden = YES;
}

- (void)showButtons {
    self.regBtn.hidden = NO;
    self.loginBtn.hidden = NO;
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

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.destinationViewController isKindOfClass:[RegLoginViewController class]]) {
        RegLoginViewController *regLoginVC = (RegLoginViewController *)segue.destinationViewController;
        regLoginVC.delegate = self;
        if ([segue.identifier isEqualToString:@"RegSegue"]) {
            regLoginVC.type = ActionTypeReg;
        } else {
            regLoginVC.type = ActionTypeLogin;
        }
    }
}

- (void)regLoginViewController:(RegLoginViewController *)viewController didSuccessWithUser:(SSOUser *)user {
    [self.navigationController popToViewController:self animated:YES];
    [self saveUser:user];
    UserViewController *userViewController = [self.storyboard instantiateViewControllerWithIdentifier:@"UserViewController"];
    userViewController.user = user;
    [self presentViewController:userViewController animated:YES completion:nil];
}

- (void)saveUser:(SSOUser *)user {
    [[NSUserDefaults standardUserDefaults] setObject:[user data] forKey:kUserDefaultLoginUser];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}


@end
