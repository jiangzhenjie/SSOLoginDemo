//
//  RegLoginViewController.h
//  SSOLoginClient
//
//  Created by jiangzhenjie on 2018/7/15.
//  Copyright © 2018年 JiangZhenjie. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef NS_ENUM(NSUInteger, ActionType) {
    ActionTypeLogin,
    ActionTypeReg,
};

@class RegLoginViewController, SSOUser;
@protocol RegLoginViewControllerDelegate <NSObject>

- (void)regLoginViewController:(RegLoginViewController *)viewController didSuccessWithUser:(SSOUser *)user;

@end

@interface RegLoginViewController : UIViewController

@property (nonatomic, assign) ActionType type;
@property (nonatomic, weak) id<RegLoginViewControllerDelegate> delegate;

@end
