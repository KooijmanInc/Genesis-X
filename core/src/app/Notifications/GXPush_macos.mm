// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <Cocoa/Cocoa.h>
#import <UserNotifications/UserNotifications.h>

#import "NotificationHandler_apple_bridge.h"
#include "src/notifications/NotificationHandler.h"

extern "C" void gx_macos_push_anchor(void) {/* just an anchor */NSLog(@"arrived");}

extern gx::NotificationHandler* gx_s_handler;

@interface GXAppDelegateProxy : NSObject<NSApplicationDelegate, UNUserNotificationCenterDelegate>
@property (nonatomic, weak) id<NSApplicationDelegate> original;
@end

@implementation GXAppDelegateProxy

#pragma mark - UNUserNotificationCenterDelegate (foreground + taps)

@end
