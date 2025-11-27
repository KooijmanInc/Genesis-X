// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <AppKit/AppKit.h>
#import <UserNotifications/UserNotifications.h>
#import <objc/runtime.h>
#include <QtCore/qglobal.h>

#include "NotificationHandler_apple_bridge.h"
#include "src/notifications/NotificationHandler.h"

extern "C" void gx_macos_push_anchor(void) {qDebug() << "macos anchor active";}

static void gx_install_hooks(void);

// --- Simple handler that will be attached to the app delegate if it doesn't
//     already implement the APNs callbacks.
@interface GXPushHandler : NSObject
@end

@implementation GXPushHandler
- (void)application:(NSApplication *)app
didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{
    // TODO: forward token into your Qt code
    NSLog(@"APNs token: %@", deviceToken);
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

- (void)application:(NSApplication *)app
didFailToRegisterForRemoteNotificationsWithError:(NSError *)error
{
    NSLog(@"APNs failed: %@", error);
}
@end

// Try to add the methods to the existing delegate class if they are missing.
// (If they already exist, we leave them alone. Swizzling can be added later if needed.)
static void gx_attach_if_missing(Class cls, SEL sel, SEL implSel, const char *types) {
    Method m = class_getInstanceMethod([GXPushHandler class], implSel);
    IMP imp = method_getImplementation(m);
    class_addMethod(cls, sel, imp, types); // no-op if method exists
}

static void gx_install_hooks_on_delegate(id delegate) {
    if (!delegate) return;
    Class cls = object_getClass(delegate);

    gx_attach_if_missing(cls,
        @selector(application:didRegisterForRemoteNotificationsWithDeviceToken:),
        @selector(application:didRegisterForRemoteNotificationsWithDeviceToken:),
        "v@:@@");

    gx_attach_if_missing(cls,
        @selector(application:didFailToRegisterForRemoteNotificationsWithError:),
        @selector(application:didFailToRegisterForRemoteNotificationsWithError:),
        "v@:@@");
}

static void gx_install_hooks(void) {
    // Make sure NSApp exists and has a delegate (Qt sets it during app init)
    if (NSApp && NSApp.delegate) {
        gx_install_hooks_on_delegate(NSApp.delegate);
    } else {
        // If we're too early, wait for app finish-launching and retry once
        [[NSNotificationCenter defaultCenter]
            addObserverForName:NSApplicationDidFinishLaunchingNotification
                        object:nil
                         queue:nil
                    usingBlock:^(__unused NSNotification *note) {
            gx_install_hooks_on_delegate(NSApp.delegate);
        }];
    }
}

// Run as soon as the binary is loaded
__attribute__((constructor))
static void gx_ctor(void) {
    gx_install_hooks();

    // Optional: ask for notification permission here (macOS 11+ supports UNUserNotificationCenter)
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    UNAuthorizationOptions opts = (UNAuthorizationOptionAlert |
                                   UNAuthorizationOptionSound |
                                   UNAuthorizationOptionBadge);
    [center requestAuthorizationWithOptions:opts completionHandler:^(__unused BOOL granted, __unused NSError *err) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [NSApp registerForRemoteNotifications];
        });
    }];
}
