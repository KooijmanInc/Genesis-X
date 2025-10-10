SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <Cocoa/Cocoa.h>
#import <UserNotifications/UserNotifications.h>
#import <TargetConditionals.h>
#include <QtCore/qglobal.h>

#import "NotificationHandler_apple_bridge.h"
#include "NotificationHandler.h"

extern "C" void gx_macos_push_anchor(void) {/* just an anchor */}

//extern "C" void gx_mac_install_push_delegate(void) {
//  dispatch_async(dispatch_get_main_queue(), ^{
//    id<NSApplicationDelegate> appDel = [NSApp delegate];
//    [UNUserNotificationCenter currentNotificationCenter].delegate = (id<UNUserNotificationCenterDelegate>)appDel;
//    NSLog(@"[GX Push macOS] installed UN center delegate on %@", appDel);
//  });
//}

#if __has_include(<FirebaseMessaging/FirebaseMessaging.h>)
#import <FirebaseMessaging/FirebaseMessaging.h>
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
@interface QNSApplicationDelegate : NSObject <NSApplicationDelegate> @end
@interface QNSApplicationDelegate (GXPush) <UNUserNotificationCenterDelegate>
@end
@implementation QNSApplicationDelegate (GXPush)
#else
@interface QCocoaApplicationDelegate : NSObject <NSApplicationDelegate> @end
//@interface QCocoaApplicationDelegate (GXPush) <UNUserNotificationCenterDelegate>
//@end
@implementation QCocoaApplicationDelegate (GXPush)
#endif
//@interface QCocoaApplicationDelegate (GXPush) @end




- (void)application:(NSApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{
#if __has_include(<FirebaseMessaging/FirebaseMessaging.h>)
  [FIRMessaging messaging].APNSToken = deviceToken;
#endif
  if (gx_s_handler) {
    const unsigned char* bytes = (const unsigned char*)deviceToken.bytes;
    NSMutableString* hex = [NSMutableString stringWithCapacity:deviceToken.length*2];
    for (NSUInteger i = 0; i < deviceToken.length; ++i) [hex appendFormat:@"%02x", bytes[i]];
    gx_s_handler->appleDidReceiveToken(QString::fromNSString(hex));
  }
}

- (void)application:(NSApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error
{
  NSLog(@"[GX] APNs registration failed: %@", error);
}

- (void)application:(NSApplication *)application didReceiveRemoteNotification:(NSDictionary*)userInfo
{
  if (gx_s_handler) {
    QString title = QString::fromNSString(userInfo[@"aps"][@"alert"][@"title"] ?: @"");
    QString body = QString::fromNSString(userInfo[@"aps"][@"alert"][@"body"] ?: @"");
    QVariantMap data;
    emit gx_s_handler->notificationReceived(title, body, data);
  }
  //completionHandler(UIBackgroundFetchResultNoData);
}

@end
