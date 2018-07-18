//
//  RegLoginViewController.m
//  SSOLoginClient
//
//  Created by jiangzhenjie on 2018/7/15.
//  Copyright © 2018年 JiangZhenjie. All rights reserved.
//

#import "RegLoginViewController.h"
#import <SSOLoginClient/Ssologin.pbrpc.h>
#import "SSODefines.h"

#include "ssologin_crypto.h"

@interface RegLoginViewController ()

@property (weak, nonatomic) IBOutlet UITextField *usernameField;
@property (weak, nonatomic) IBOutlet UITextField *passwordField;
@property (weak, nonatomic) IBOutlet UILabel *errorLabel;
@property (weak, nonatomic) IBOutlet UIButton *submitBtn;

@property (nonatomic, strong) UIActivityIndicatorView *indicatorView;

@end

@implementation RegLoginViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self setupViews];
}

- (void)setupViews {
    NSString *title = self.type == ActionTypeReg ? @"注册" : @"登录";
    self.title = title;
    [self.submitBtn setTitle:title forState:UIControlStateNormal];
    self.submitBtn.enabled = NO;
}

- (void)showError:(NSError *)error {
    self.errorLabel.text = error.localizedDescription;
    self.errorLabel.hidden = NO;
}

- (void)showLoading {
    if (self.indicatorView == nil) {
        self.indicatorView = [[UIActivityIndicatorView alloc] init];
        self.indicatorView.activityIndicatorViewStyle = UIActivityIndicatorViewStyleGray;
        self.indicatorView.center = CGPointMake(self.submitBtn.center.x, self.submitBtn.center.y + 40);
        [self.view addSubview:self.indicatorView];
    }
    [self.indicatorView startAnimating];
    self.indicatorView.hidden = NO;
}

- (void)hideLoading {
    [self.indicatorView stopAnimating];
    self.indicatorView.hidden = YES;
}

- (IBAction)textFieldValueChanged:(UITextField *)sender {
    self.submitBtn.enabled = self.usernameField.text.length != 0 && self.passwordField.text.length != 0;
}

- (IBAction)pressSubmitBtn:(UIButton *)sender {
    SSOUserService *userService = [[SSOUserService alloc] initWithHost:kHostAddress];
    
    SSOCredential *credential = [SSOCredential message];
    credential.username = self.usernameField.text;
    credential.password = [[self class] encryptPassword:self.passwordField.text];
    
    [self showLoading];
    if (self.type == ActionTypeReg) {
        [userService registerWithRequest:credential handler:^(SSOUser * _Nullable response, NSError * _Nullable error) {
            [self hideLoading];
            if (error == nil) {
                NSLog(@"注册成功");
                [self.delegate regLoginViewController:self didSuccessWithUser:response];
            } else {
                [self showError:error];
            }
        }];
    } else {
        [userService loginWithRequest:credential handler:^(SSOUser * _Nullable response, NSError * _Nullable error) {
            [self hideLoading];
            if (error == nil) {
                NSLog(@"登录成功");
                [self.delegate regLoginViewController:self didSuccessWithUser:response];
            } else {
                [self showError:error];
            }
        }];
    }
}

+ (NSString *)encryptPassword:(NSString *)password {
    
    NSString *path = [[NSBundle mainBundle] pathForResource:@"public" ofType:@"pem"];
    
    const char *plainText = [password cStringUsingEncoding:NSUTF8StringEncoding];
    size_t plaintextLen = password.length;
    const char *cPath = [path cStringUsingEncoding:NSUTF8StringEncoding];
    unsigned char ciphertext[1024];
    size_t ciphertext_len;
    
    int ret = rsa_public_encrypt((unsigned char *)plainText, plaintextLen, cPath, ciphertext, &ciphertext_len);
    if (ret == 0) {
        NSData *data = [NSData dataWithBytes:ciphertext length:ciphertext_len];
        return [data base64EncodedStringWithOptions:NSDataBase64EncodingEndLineWithCarriageReturn];
    }
    
    return nil;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

@end
