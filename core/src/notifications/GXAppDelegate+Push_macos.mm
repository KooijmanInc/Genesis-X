// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#import <AppKit/AppKit.h>
#import <TargetConditionals.h>
#import <UserNotifications/UserNotifications.h>
#include <QtCore/qglobal.h>

#include "NotificationHandler_apple_bridge.h"
#include "NotificationHandler.h"

extern "C" void gx_macos_push_anchor(void) {qDebug() << "macos anchor active";}

@interface QNSApplicationDelegate : NSObject <NSApplicationDelegate> @end
@interface QNSApplicationDelegate (GXPush) <UNUserNotificationCenterDelegate> @end

@implementation QNSApplicationDelegate (GXPush)

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
