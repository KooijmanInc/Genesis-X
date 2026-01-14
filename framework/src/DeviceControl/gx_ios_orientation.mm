// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <UIKit/UIKit.h>
#include <stdatomic.h>
#include <dispatch/dispatch.h>

static atomic_int g_gxOrientationMask = 0;

static UIViewController *gx_topViewController(UIViewController *vc)
{
    while (vc.presentedViewController) vc = vc.presentedViewController;

    if ([vc isKindOfClass:[UINavigationController class]]) {
        UIViewController *v = ((UINavigationController *)vc).visibleViewController;
        return v ?: vc;
    }
    if ([vc isKindOfClass:[UITabBarController class]]) {
        UIViewController *v = ((UITabBarController *)vc).selectedViewController;
        return v ?: vc;
    }
    return vc;
}

static UIWindow *gx_firstWindow(void)
{
  UIApplication *app = UIApplication.sharedApplication;

  for (UIScene *scene in app.connectedScenes) {
    if (scene.activationState != UISceneActivationStateForegroundActive) continue;

    if (![scene isKindOfClass:[UIWindowScene class]]) continue;

    UIWindowScene *ws = (UIWindowScene *)scene;

    for (UIWindow *w in ws.windows) {
      if (w.isKeyWindow) return w;
    }

    if (ws.windows.count > 0) return ws.windows.firstObject;
  }

  return nil;
  // NSArray<UIWindow *> *wins = UIApplication.sharedApplication.windows;
  // NSArray<UIWindow *> *wins = UIWindowScene.window;

  // return wins.count > 0 ? wins[0] : nil;
}

static void gx_requestRotationUpdate(UIInterfaceOrientationMask desiredMask)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        UIWindow *w = gx_firstWindow();
        if (!w) return;

UIViewController *root = w.rootViewController;
        UIViewController *top  = root ? gx_topViewController(root) : nil;

        // iOS 16+: best-effort "push" using scene geometry update
        if (@available(iOS 16.0, *)) {
            UIWindowScene *ws = w.windowScene;
            if (ws) {
                UIWindowSceneGeometryPreferencesIOS *prefs =
                    [[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:desiredMask];

                [ws requestGeometryUpdateWithPreferences:prefs
                                           errorHandler:^(NSError * _Nonnull error) {
                    // Optional debug:
                    // NSLog(@"requestGeometryUpdate error: %@", error);
                }];
            }
        }

        if (top) {
            [top setNeedsUpdateOfSupportedInterfaceOrientations];
        }
        // UIViewController *root = w.rootViewController;
        // UIViewController *top  = root ? gx_topViewController(root) : nil;

        // // iOS 16+: request geometry update on the window scene (best-effort “push”)
        // if (@available(iOS 16.0, *)) {
        //     UIWindowScene *ws = w.windowScene;
        //     if (ws) {
        //         UIWindowSceneGeometryPreferencesIOS *prefs =
        //             [[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:desiredMask];

        //         [ws requestGeometryUpdateWithPreferences:prefs errorHandler:^(NSError * _Nonnull error) {
        //             // Optional: log if you want
        //             // NSLog(@"Geometry update error: %@", error);
        //         }];
        //     }
        // }

        // This is the *replacement* Apple points to. IMPORTANT: instance method, not class method.
        // if (top) {
        //     [top setNeedsUpdateOfSupportedInterfaceOrientations];
        // }

        // No attemptRotationToDeviceOrientation() (deprecated iOS 16)
    });
}



// static UIViewController *gx_topViewController(UIViewController *vc)
// {
//     while (vc.presentedViewController) vc = vc.presentedViewController;

//     if ([vc isKindOfClass:[UINavigationController class]]) {
//         UIViewController *v = ((UINavigationController *)vc).visibleViewController;
//         return v ?: vc;
//     }
//     if ([vc isKindOfClass:[UITabBarController class]]) {
//         UIViewController *v = ((UITabBarController *)vc).selectedViewController;
//         return v ?: vc;
//     }
//     return vc;
// }


// static void gx_requestRotationUpdate(void)
// {
//   dispatch_async(dispatch_get_main_queue(), ^{
//     UIWindow *w = gx_firstWindow();
//     UIViewController *root = w.rootViewController;
//     if (root) {
//       [root setNeedsUpdateOfSupportedInterfaceOrientations];
//     }
//     // [UIViewController attemptRotationToDeviceOrientation];
//     [UIViewController setNeedsUpdateOfSupportedInterfaceOrientations];
//   });
// }

extern "C" void gx_ios_set_orientation_portrait(void)
{
  atomic_store(&g_gxOrientationMask, (int)UIInterfaceOrientationMaskPortrait);
  gx_requestRotationUpdate(UIInterfaceOrientationMaskPortrait);
}

extern "C" void gx_ios_set_orientation_landscape(void)
{
  atomic_store(&g_gxOrientationMask, (int)UIInterfaceOrientationMaskLandscape);
  gx_requestRotationUpdate(UIInterfaceOrientationMaskLandscape);
}

extern "C" void gx_ios_clear_orientation_override(void)
{
  atomic_store(&g_gxOrientationMask, 0);
  gx_requestRotationUpdate(UIInterfaceOrientationMaskAllButUpsideDown);
}

@interface QIOSApplicationDelegate
@end

@interface QIOSApplicationDelegate (GXOrientation)
@end

@implementation QIOSApplicationDelegate (GXOrientation)

- (UIInterfaceOrientationMask)application:(UIApplication *)application
  supportedInterfaceOrientationsForWindow:(UIWindow *) window
{
  int mask = atomic_load(&g_gxOrientationMask);

  if (mask == 0) return UIInterfaceOrientationMaskAllButUpsideDown;

  return (UIInterfaceOrientationMask)mask;
}

@end
