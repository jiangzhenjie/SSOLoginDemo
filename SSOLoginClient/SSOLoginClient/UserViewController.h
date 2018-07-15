//
//  UserViewController.h
//  SSOLoginClient
//
//  Created by jiangzhenjie on 2018/7/15.
//  Copyright © 2018年 JiangZhenjie. All rights reserved.
//

#import <UIKit/UIKit.h>

@class SSOUser;
@interface UserViewController : UIViewController

@property (nonatomic, strong) SSOUser *user;

- (instancetype)initWithUser:(SSOUser *)user;

@end
