// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import <TargetConditionals.h>
#import <UserNotifications/UserNotifications.h>
#include <QtCore/qglobal.h>

#if GX_HAVE_FIREBASE
  #import <FirebaseMessaging.h>
#else
    #undef GX_HAVE_FIREBASE
    #define GX_HAVE_FIREBASE 0
#endif

#import "NotificationHandler_apple_bridge.h"
#include <GenesisX/Notifications/NotificationHandler.h>

extern "C" void gx_ios_push_anchor(void) {qDebug() << "ios anchor active";}

@interface QIOSApplicationDelegate : UIResponder <UIApplicationDelegate> @end

@interface QIOSApplicationDelegate (GXPush) <UNUserNotificationCenterDelegate> @end

@implementation QIOSApplicationDelegate (GXPush)

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{qInfo() << "getting to ios getting token";
#if GX_HAVE_FIREBASE
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
