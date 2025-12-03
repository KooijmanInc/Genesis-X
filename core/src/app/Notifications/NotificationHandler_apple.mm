// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <TargetConditionals.h>
#import <UserNotifications/UserNotifications.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

#import <objc/message.h>

#if !defined(GX_HAVE_FIREBASE)
#define GX_HAVE_FIREBASE 0
#endif

#if GX_HAVE_FIREBASE
  #import <FirebaseCore.h>
  #import <FirebaseMessaging.h>
#else
    #undef GX_HAVE_FIREBASE
    #define GX_HAVE_FIREBASE 0
#endif

#include "NotificationHandler_apple_bridge.h"
#include <GenesisX/Notifications/NotificationHandler.h>
#include <QCoreApplication>
#include <QVariantMap>
#include <QDebug>

#if TARGET_OS_IOS
extern "C" void gx_ios_push_anchor(void);
#else
extern "C" void gx_macos_push_anchor(void);
#endif

gx::app::notifications::NotificationHandler* gx_s_handler = nullptr;

using namespace gx;

@interface GXUNDelegate : NSObject<UNUserNotificationCenterDelegate
#if GX_HAVE_FIREBASE
, FIRMessagingDelegate
#endif
>
@property (nonatomic, assign) gx::app::notifications::NotificationHandler* owner;
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
    withCompleteHandler:(void (^) (void))completionHandler
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

#if GX_HAVE_FIREBASE
-(void)messaging:(FIRMessaging *) messaging
  didReceiveRegistrationToken:(NSString *)fcmToken
{
  Q_UNUSED(messaging);

  const QString qToken = QString::fromNSString(fcmToken);

  if (self.owner) {
    self.owner->appleDidReceiveToken(qToken);
  }
  if (gx_s_handler && gx_s_handler != self.owner) {
    gx_s_handler->appleDidReceiveToken(qToken);
  }
}
#endif

@end

namespace {
static GXUNDelegate* s_delegate = nil;
static dispatch_once_t s_once;
}

void gx::app::notifications::NotificationHandler::appleInitialize(const QVariantMap &options)
{
  Q_UNUSED(options);

  gx_s_handler = this;

#if TARGET_OS_IOS
  gx_ios_push_anchor();
#else
  gx_macos_push_anchor();
#endif

  qInfo() << "[GX Notify] appleInitialize() called. GX_HAVE_FIREBASE =" << GX_HAVE_FIREBASE;

  dispatch_once(&s_once, ^{
    UNUserNotificationCenter* center = UNUserNotificationCenter.currentNotificationCenter;
    s_delegate = [GXUNDelegate new];
    s_delegate.owner = this;
    center.delegate = s_delegate;

    UNAuthorizationOptions opts = (UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge);
    [center requestAuthorizationWithOptions:opts completionHandler:^(bool granted, NSError * _Nullable error) {
      if (error) {
        qWarning() << "[GX Notify] Apple authorization error:" << QString::fromNSString(error.localizedDescription);
      } else {
        qInfo() << "[GX Notify] Apple notifications authorization:" << (granted ? "granted" : "denied");
      }
    }];

#if GX_HAVE_FIREBASE
  if ([FIRApp defaultApp] == nil) {
    qInfo() << "[GX Notify] Calling [FIRApp configure]";
    [FIRApp configure];
  } else {
    qInfo() << "[GX Notify] FIRApp already configured";
  }

  [FIRMessaging messaging].delegate = s_delegate;
#endif

#if TARGET_OS_IOS
    dispatch_async(dispatch_get_main_queue(), ^{
      [[UIApplication sharedApplication] registerForRemoteNotifications];
    });
#else
    qInfo() << "[GX Notify] also setting APNs for macOS";
    dispatch_async(dispatch_get_main_queue(), ^{
      [NSApp registerForRemoteNotifications];
    });
#endif
  });
}

void gx::app::notifications::NotificationHandler::showAppleNotification(const QString &title, const QString &body, int msec)
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
