// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <TargetConditionals.h>
#import <UserNotifications/UserNotifications.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

// #if __has_include(<FirebaseCore/FirebaseCore.h>)
// #import <FirebaseCore/FirebaseCore.h>
// #endif

// #if __has_include(<FirebaseMessaging/FirebaseMessaging.h>)
// #import <FirebaseMessaging/FirebaseMessaging.h>
// #endif

#include "NotificationHandler_apple_bridge.h"
#include "NotificationHandler.h"
// #include <QCoreApplication>
// #include <QVariantMap>
// #include <QDebug>

// #if TARGET_OS_IOS
// extern "C" void gx_ios_push_anchor(void);
// //extern "C" void gx_ios_install_push_delegate(void);)
// #else
// extern "C" void gx_macos_push_anchor(void);
// //extern "C" void gx_macos_install_push_delegate(void);
// #endif

gx::NotificationHandler* gx_s_handler = nullptr;

using namespace gx;

@interface GXUNDelegate : NSObject<UNUserNotificationCenterDelegate>
// #if __has_include(<FirebaseMessaging/FirebaseMessaging.h>)
// , FIRMessagingDelegate
// #endif
@property (nonatomic, assign) gx::NotificationHandler* owner;
@end

@implementation GXUNDelegate
- (void)userNotificationCenter:(UNUserNotificationCenter *)center
  willPresentNotification:(UNNotification *)notification
    withCompleteHandler:(void (^)(UNNotificationPresentationOptions options))completionHandler
{
#if TARGET_OS_IPHONE
  completionHandler(UNNotificationPresentationOptionBanner | UNNotificationPresentationOptionSound);
#else
  completionHandler(UNNotificationPresentationOptionBanner | UNNotificationPresentationOptionSound | UNNotificationPresentationOptionList);
#endif
  if (self.owner) {
    const QString title = QString::fromNSString(notification.request.content.title);
    const QString body = QString::fromNSString(notification.request.content.body);
    emit self.owner->notificationReceived(title, body, {});
  }
  if (gx_s_handler) {
    const QString title = QString::fromNSString(notification.request.content.title);
    const QString body = QString::fromNSString(notification.request.content.body);
    emit gx_s_handler->notificationReceived(title, body, {});
  }
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
didReceiveNotificationResponse:(UNNotificationResponse *)response
  withCompletionHandler:(void (^)(void))completionHandler
{
  if (self.owner) {
    const QString title = QString::fromNSString(response.notification.request.content.title);
    const QString body = QString::fromNSString(response.notification.request.content.body);
    emit self.owner->notificationReceived(title, body, {});
  }
  if (gx_s_handler) {
    const QString title = QString::fromNSString(response.notification.request.content.title);
    const QString body = QString::fromNSString(response.notification.request.content.body);
    emit gx_s_handler->notificationReceived(title, body, {});
  }
  completionHandler();
}

// #if __has_include(<FirebaseMessaging/FirebaseMessaging.h>)
// - (void)messaging:(FIRMessaging *)messaging didReceiveRegistrationToken:(NSString *)fcmToken
// {
//   if (gx_s_handler) {qInfo() << "getting to receive token????????";
//     emit gx_s_handler->tokenChanged(QString::fromNSString(fcmToken));
//     qInfo() << "[GX Notigy] FCM token:" << QString::fromNSString(fcmToken);
//   }
// }
// #endif
@end

// namespace {
// static GXUNDelegate* s_delegate = nil;
// static dispatch_once_t s_delegateOnce;
// }

void NotificationHandler::appleInitialize(const QVariantMap &options)
{
  Q_UNUSED(options);
  gx_s_handler = this;

#if TARGET_OS_IOS
  gx_ios_push_anchor();
  //gx_ios_install_push_delegate();
#else
  gx_macos_push_anchor();
  //gx_macos_install_push_delegate();
#endif

  dispatch_once(&s_delegateOnce, ^{
    s_delegate = [GXUNDelegate new];
    s_delegate.owner = this;
    //s_delegate.gx_s_handler = this;

    UNUserNotificationCenter* center = UNUserNotificationCenter.currentNotificationCenter;
    center.delegate = s_delegate;

    UNAuthorizationOptions opts = (UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge);
    [center requestAuthorizationWithOptions:opts completionHandler:^(bool granted, NSError * _Nullable error) {
      if (error) {
        qWarning() << "[GX Notify] Apple authorization error:" << QString::fromNSString(error.localizedDescription);
      } else {
        qInfo() << "[GX Notify] Apple notifications authorization" << (granted ? "granted" : "denied");
      }
    }];

#if __has_include(<FirebaseMessaging/FirebaseMessaging.h>)
    static dispatch_once_t onceFirebase;
    dispatch_once(&onceFirebase, ^{
      if ([FIRApp defaultApp] == nil) {
        [FIRApp configure];
      }
      [FIRMessaging messaging].delegate = s_delegate;
    });
#endif

#if TARGET_OS_IPHONE
    dispatch_async(dispatch_get_main_queue(), ^{qInfo() << "arived at target";
      [[UIApplication sharedApplication] registerForRemoteNotifications];
    });
#else
    dispatch_async(dispatch_get_main_queue(), ^{
      [NSApp registerForRemoteNotifications];
    });
#endif
  });
}

void NotificationHandler::showAppleNotification(const QString& title, const QString& body, int msec)
{
  UNUserNotificationCenter* center = UNUserNotificationCenter.currentNotificationCenter;

  UNMutableNotificationContent* content = [UNMutableNotificationContent new];
  content.title = title.toNSString();
  content.body = body.toNSString();

  UNNotificationTrigger* trigger = nil;
  if (msec > 0) {
    NSTimeInterval seconds = MAX(1.0, msec / 1000.0);
    trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:seconds repeats:NO];
  }

  NSString* ident = [NSString stringWithFormat:@"gx.local.%@", [[NSUUID UUID] UUIDString]];
  UNNotificationRequest* req = [UNNotificationRequest requestWithIdentifier:ident content:content trigger:trigger];

  [center addNotificationRequest:req withCompletionHandler:^(NSError * _Nullable error) {
    if (error) {
      qWarning() << "[GX Notify] addNotificationRequerst failed:" << QString::fromNSString(error.localizedDescription);
    }
  }];
}

