// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <UIKit/UIKit.h>
#import <PhotosUI/PhotosUI.h>

#include <QUrl>
#include <QString>
#include <QMetaType>
#include <QMetaObject>

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

static UIViewController *gx_presentingController(void)
{
    UIApplication *app = UIApplication.sharedApplication;

    for (UIScene *scene in app.connectedScenes) {
        if (scene.activationState != UISceneActivationStateForegroundActive) continue;
        if (![scene isKindOfClass:[UIWindowScene class]]) continue;

        UIWindowScene *ws = (UIWindowScene *)scene;

        UIWindow *w = nil;
        for (UIWindow *win in ws.windows) { if (win.isKeyWindow) { w = win; break; } }
        if (!w && ws.windows.count > 0) w = ws.windows.firstObject;
        if (!w) continue;

        UIViewController *root = w.rootViewController;
        if (!root) continue;
        return gx_topViewController(root);
    }
    return nil;
}

// Write UIImage -> temp file, return path
static NSString *gx_writeImageToTemp(UIImage *img)
{
    if (!img) return nil;

    NSData *data = UIImageJPEGRepresentation(img, 0.92);
    if (!data) return nil;

    NSString *fn = [NSString stringWithFormat:@"gx_pick_%@.jpg", NSUUID.UUID.UUIDString];
    NSString *path = [NSTemporaryDirectory() stringByAppendingPathComponent:fn];

    if ([data writeToFile:path atomically:YES]) return path;
    return nil;
}

static void gx_emitImageReady(QObject *ctx, NSString *path)
{
    if (!ctx || !path) return;

    QString qpath = QString::fromNSString(path);
    QUrl url = QUrl::fromLocalFile(qpath);

    QMetaObject::invokeMethod(ctx, [ctx, url]() {
        QMetaObject::invokeMethod(ctx, "imageReady",
                                  Qt::QueuedConnection,
                                  Q_ARG(QUrl, url));
    }, Qt::QueuedConnection);
}

static void gx_emitError(QObject *ctx, NSString *msg)
{
    if (!ctx) return;
    QString qmsg = QString::fromNSString(msg ?: @"Unknown error");

    QMetaObject::invokeMethod(ctx, [ctx, qmsg]() {
        QMetaObject::invokeMethod(ctx, "error",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, qmsg));
    }, Qt::QueuedConnection);
}

@interface GXCameraDelegate : NSObject <UIImagePickerControllerDelegate, UINavigationControllerDelegate>
@property(nonatomic, assign) QObject *ctx;
@end

@implementation GXCameraDelegate
- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
    UIImage *img = info[UIImagePickerControllerOriginalImage];
    NSString *path = gx_writeImageToTemp(img);

    [picker dismissViewControllerAnimated:YES completion:nil];

    if (path) gx_emitImageReady(self.ctx, path);
    else gx_emitError(self.ctx, @"Failed to save camera image");
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    [picker dismissViewControllerAnimated:YES completion:nil];
    gx_emitError(self.ctx, @"Cancelled");
}
@end

@interface GXPickerDelegate : NSObject <PHPickerViewControllerDelegate>
@property(nonatomic, assign) QObject *ctx;
@end

@implementation GXPickerDelegate
- (void)picker:(PHPickerViewController *)picker didFinishPicking:(NSArray<PHPickerResult *> *)results
{
    [picker dismissViewControllerAnimated:YES completion:nil];

    if (results.count == 0) {
        gx_emitError(self.ctx, @"Cancelled");
        return;
    }

    PHPickerResult *r = results.firstObject;
    NSItemProvider *prov = r.itemProvider;

    if (![prov canLoadObjectOfClass:UIImage.class]) {
        gx_emitError(self.ctx, @"Unsupported selection");
        return;
    }

    [prov loadObjectOfClass:UIImage.class completionHandler:^(UIImage *img, NSError *err) {
        if (err || !img) {
            gx_emitError(self.ctx, err.localizedDescription ?: @"Failed to load image");
            return;
        }
        NSString *path = gx_writeImageToTemp(img);
        if (path) gx_emitImageReady(self.ctx, path);
        else gx_emitError(self.ctx, @"Failed to save selected image");
    }];
}
@end

// Keep delegates alive while presented
static GXCameraDelegate *s_camDelegate = nil;
static GXPickerDelegate *s_pickerDelegate = nil;

void gx_ios_photo_picker_open_camera(QObject *ctx)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController *vc = gx_presentingController();
        if (!vc) { gx_emitError(ctx, @"No presenting view controller"); return; }

        if (![UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera]) {
            gx_emitError(ctx, @"Camera not available");
            return;
        }

        UIImagePickerController *picker = [UIImagePickerController new];
        picker.sourceType = UIImagePickerControllerSourceTypeCamera;
        picker.modalPresentationStyle = UIModalPresentationFullScreen;

        s_camDelegate = [GXCameraDelegate new];
        s_camDelegate.ctx = ctx;
        picker.delegate = s_camDelegate;

        [vc presentViewController:picker animated:YES completion:nil];
    });
}

void gx_ios_photo_picker_open_gallery(QObject *ctx)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController *vc = gx_presentingController();
        if (!vc) { gx_emitError(ctx, @"No presenting view controller"); return; }

        if (@available(iOS 14.0, *)) {
            PHPickerConfiguration *cfg = [PHPickerConfiguration new];
            cfg.selectionLimit = 1;
            cfg.filter = [PHPickerFilter imagesFilter];

            PHPickerViewController *picker = [[PHPickerViewController alloc] initWithConfiguration:cfg];
            picker.modalPresentationStyle = UIModalPresentationFullScreen;

            s_pickerDelegate = [GXPickerDelegate new];
            s_pickerDelegate.ctx = ctx;
            picker.delegate = s_pickerDelegate;

            [vc presentViewController:picker animated:YES completion:nil];
        } else {
            gx_emitError(ctx, @"Gallery picker requires iOS 14+");
        }
    });
}
