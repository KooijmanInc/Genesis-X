// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import <UserNotifications/UserNotifications.h>

#import "NotificationHandler_apple_bridge.h"
#include "src/notifications/NotificationHandler.h"

#if __has_include(<FirebaseMessaging/FirebaseMessaging.h>)
#import <FirebaseMessaging/FirebaseMessaging.h>
#endif

@interface QIOSApplicationDelegate : UIResponder <UIApplicationDelegate> @end

//@interface QIOSApplicationDelegate (GXPush) @end
@interface QIOSApplicationDelegate (GXPush) <UNUserNotificationCenterDelegate>
@end

extern "C" void gx_ios_push_anchor(void) {/* just an anchor */}

extern "C" void gx_ios_install_push_delegate(void) {
  dispatch_async(dispatch_get_main_queue(), ^{
    id<UIApplicationDelegate> appDel = [UIApplication sharedApplication].delegate;
    [UNUserNotificationCenter currentNotificationCenter].delegate = (id<UNUserNotificationCenterDelegate>)appDel;
    NSLog(@"[GX Push] installed UN center delegate on %@", appDel);
  });
}

@implementation QIOSApplicationDelegate (GXPush)

+ (void)load {NSLog(@"[GX Push] +load reached in category");}

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{qInfo() << "getting to ios getting token";
#if __has_include(<FirebaseMessaging/FirebaseMessaging.h>)
  [FIRMessaging messaging].APNSToken = deviceToken;
#endif

  if (gx_s_handler) {
    const unsigned char* bytes = (const unsigned char*)deviceToken.bytes;
    NSMutableString* hex = [NSMutableString stringWithCapacity:deviceToken.length*2];
    for (NSUInteger i = 0; i < deviceToken.length; ++i) [hex appendFormat:@"%02x", bytes[i]];
    emit gx_s_handler->tokenChanged(QString::fromNSString(hex));
  }
}

- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary*)userInfo
  fetchCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler
{
  if (gx_s_handler) {
    QString title = QString::fromNSString(userInfo[@"aps"][@"alert"][@"title"] ?: @"");
    QString body = QString::fromNSString(userInfo[@"aps"][@"alert"][@"body"] ?: @"");
    QVariantMap data;
    emit gx_s_handler->notificationReceived(title, body, data);
  }
  completionHandler(UIBackgroundFetchResultNoData);
}

- (void)application:(UIApplication *)app
didFailToRegisterForNotificationsWithError:(NSError *)error
{
  NSLog(@"[GX Notify] didFailToRegisterForRemoteNotificationsWithError: %@", error);
}

@end
