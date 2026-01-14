// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#import <Foundation/Foundation.h>
#import <LocalAuthentication/LocalAuthentication.h>
#import <Security/Security.h>

#include <QVariant>
#include <QVariantMap>
#include <QString>
#include <QMetaObject>

namespace {
static NSString *kService = @"io.genesisx.app";
static NSString *kAccount = @"biometric_login_token";

// Helper: dispatch result back to Qt object thread
static void emitAuthenticated(QObject *ctx, int code, const QString &msg)
{
    if (!ctx) return;
    QMetaObject::invokeMethod(ctx, [ctx, code, msg]() {
        QMetaObject::invokeMethod(ctx, "authenticated",
                                 Qt::QueuedConnection,
                                 Q_ARG(int, code),
                                 Q_ARG(QString, msg));
    }, Qt::QueuedConnection);
}

static QVariantMap makeResult(int code, const QString &message, const QString &token = QString())
{
    QVariantMap m;
    m.insert(QStringLiteral("code"), code);
    m.insert(QStringLiteral("message"), message);
    if (!token.isNull())
        m.insert(QStringLiteral("token"), token);
    return m;
}

// Map iOS LAError to your BiometricsResult codes (adjust to your enum)
static int mapLAError(NSError *err)
{
    if (!err) return 0;

    switch (err.code) {
        case LAErrorAuthenticationFailed: return 2;         // e.g. Failed
        case LAErrorUserCancel:           return 3;         // Cancelled
        case LAErrorUserFallback:         return 3;
        case LAErrorBiometryNotAvailable: return 4;         // NotAvailable
        case LAErrorBiometryNotEnrolled:  return 5;         // NotEnrolled
        case LAErrorBiometryLockout:      return 6;         // LockedOut
        default:                          return 1;         // Internal
    }
}

static bool canBiometricAuth(LAContext *ctx, NSError **outErr)
{
    return [ctx canEvaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics error:outErr];
}

static NSDictionary *baseKeychainQuery(void)
{
    return @{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: kService,
        (__bridge id)kSecAttrAccount: kAccount
    };
}
} // namespace

bool gx_app_biometrics_available_ios()
{
    LAContext *ctx = [LAContext new];
    NSError *err = nil;
    return canBiometricAuth(ctx, &err);
}

int gx_app_biometrics_status_ios()
{
    LAContext *ctx = [LAContext new];
    NSError *err = nil;
    if (canBiometricAuth(ctx, &err))
        return 0; // OK/Available (match your enum)

    // Map common cases (adjust to your BiometricsResult)
    if (!err) return 1;
    if (err.code == LAErrorBiometryNotAvailable) return 4;
    if (err.code == LAErrorBiometryNotEnrolled)  return 5;
    if (err.code == LAErrorBiometryLockout)      return 6;
    return 1;
}

// Async authenticate: returns immediately with a "pending" map, emits authenticated later
QVariant gx_app_biometrics_authenticate_ios(const QString &reason, QObject *ctx)
{
    LAContext *la = [LAContext new];
    la.localizedFallbackTitle = @""; // hide "Enter Password" fallback if you want strict biometrics

    NSError *canErr = nil;
    if (!canBiometricAuth(la, &canErr)) {
        const int code = mapLAError(canErr);
        const QString msg = QString::fromNSString(canErr.localizedDescription ?: @"Biometrics not available");
        emitAuthenticated(ctx, code, msg);
        return makeResult(code, msg);
    }

    NSString *nsReason = reason.isEmpty()
        ? @"Authenticate"
        : reason.toNSString();

    [la evaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics
       localizedReason:nsReason
                 reply:^(BOOL success, NSError * _Nullable error) {

        if (success) {
            emitAuthenticated(ctx, 0, QStringLiteral("OK"));
        } else {
            const int code = mapLAError(error);
            const QString msg = QString::fromNSString(error.localizedDescription ?: @"Authentication failed");
            emitAuthenticated(ctx, code, msg);
        }
    }];

    // immediate return (optional)
    return makeResult(0, QStringLiteral("started"));
}

bool gx_app_biometrics_has_token_ios(QObject * /*ctx*/)
{
    NSMutableDictionary *q = [baseKeychainQuery() mutableCopy];
    q[(__bridge id)kSecMatchLimit] = (__bridge id)kSecMatchLimitOne;
    q[(__bridge id)kSecReturnData] = @NO;

    OSStatus st = SecItemCopyMatching((__bridge CFDictionaryRef)q, nil);
    return (st == errSecSuccess);
}

bool gx_app_biometrics_clear_token_ios(QObject * /*ctx*/)
{
    NSDictionary *q = baseKeychainQuery();
    OSStatus st = SecItemDelete((__bridge CFDictionaryRef)q);
    return (st == errSecSuccess || st == errSecItemNotFound);
}

// Store token protected by biometrics (biometry current set)
QVariant gx_app_biometrics_store_token_ios(const QString &token,
                                                     const QString &reason,
                                                     QObject *ctx)
{
    // Return immediately; do actual work async to avoid main-thread stalls.
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        // Clear any existing (this one is quick, but keep it off-main too)
        gx_app_biometrics_clear_token_ios(ctx);

        NSData *data = [token.toNSString() dataUsingEncoding:NSUTF8StringEncoding];

        CFErrorRef acErr = nil;
        SecAccessControlRef ac =
            SecAccessControlCreateWithFlags(kCFAllocatorDefault,
                                            kSecAttrAccessibleWhenUnlockedThisDeviceOnly,
                                            kSecAccessControlBiometryCurrentSet,
                                            &acErr);

        if (!ac) {
            QString msg = QStringLiteral("Failed to create access control");
            // emit authenticated on Qt thread
            QMetaObject::invokeMethod(ctx, [ctx, msg]() {
                QMetaObject::invokeMethod(ctx, "authenticated",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, 1),
                                          Q_ARG(QString, msg));
            }, Qt::QueuedConnection);
            return;
        }

        LAContext *la = [LAContext new];
        NSString *nsReason = reason.isEmpty() ? @"Unlock" : reason.toNSString();

        NSMutableDictionary *q = [baseKeychainQuery() mutableCopy];
        q[(__bridge id)kSecValueData] = data;
        q[(__bridge id)kSecUseAuthenticationContext] = la;
        // q[(__bridge id)kSecUseOperationPrompt] = nsReason;
        q[(__bridge id)kSecAttrAccessControl] = (__bridge id)ac;

        OSStatus st = SecItemAdd((__bridge CFDictionaryRef)q, nil);
        CFRelease(ac);

        if (st == errSecSuccess) {
            QString msg = QStringLiteral("Token stored");
            QMetaObject::invokeMethod(ctx, [ctx, msg]() {
                // If you have loginTokenChanged in Biometrics, you can emit it from C++ after store returns,
                // or do it here if you expose it. For now mirror Android: authenticated is enough.
                QMetaObject::invokeMethod(ctx, "authenticated",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, 0),
                                          Q_ARG(QString, msg));
            }, Qt::QueuedConnection);
            return;
        }

        // Map common Keychain errors a bit nicer (optional)
        int code = 1;
        QString msg;
        if (st == errSecUserCanceled) { code = 3; msg = QStringLiteral("User cancelled"); }
        else if (st == errSecAuthFailed) { code = 2; msg = QStringLiteral("Authentication failed"); }
        else { msg = QStringLiteral("Keychain store failed (%1)").arg((int)st); }

        QMetaObject::invokeMethod(ctx, [ctx, code, msg]() {
            QMetaObject::invokeMethod(ctx, "authenticated",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, code),
                                      Q_ARG(QString, msg));
        }, Qt::QueuedConnection);
    });

    // Immediate return to QML
    return QVariantMap{
        { "code", 0 },
        { "message", "started" }
    };
}

// extern "C" QVariant gx_app_biometrics_store_token_ios(const QString &token,
//                                                      const QString &reason,
//                                                      QObject *ctx)
// {
//     // Clear any existing
//     gx_app_biometrics_clear_token_ios(ctx);

//     NSData *data = [token.toNSString() dataUsingEncoding:NSUTF8StringEncoding];

//     CFErrorRef acErr = nil;
//     SecAccessControlRef ac =
//         SecAccessControlCreateWithFlags(kCFAllocatorDefault,
//                                         kSecAttrAccessibleWhenUnlockedThisDeviceOnly,
//                                         kSecAccessControlBiometryCurrentSet,
//                                         &acErr);

//     if (!ac) {
//         QString msg = QStringLiteral("Failed to create access control");
//         emitAuthenticated(ctx, 1, msg);
//         return makeResult(1, msg);
//     }

//     LAContext *la = [LAContext new];
//     NSString *nsReason = reason.isEmpty() ? @"Unlock" : reason.toNSString();

//     NSMutableDictionary *q = [baseKeychainQuery() mutableCopy];
//     q[(__bridge id)kSecValueData] = data;
//     q[(__bridge id)kSecUseAuthenticationContext] = la;
//     q[(__bridge id)kSecUseOperationPrompt] = nsReason;
//     q[(__bridge id)kSecAttrAccessControl] = (__bridge id)ac;

//     OSStatus st = SecItemAdd((__bridge CFDictionaryRef)q, nil);
//     CFRelease(ac);

//     if (st == errSecSuccess) {
//         emitAuthenticated(ctx, 0, QStringLiteral("Token stored"));
//         // you probably also want to clear m_tokenOpInFlight back in C++ when you receive signal
//         return makeResult(0, QStringLiteral("Token stored"));
//     }

//     QString msg = QStringLiteral("Keychain store failed (%1)").arg((int)st);
//     emitAuthenticated(ctx, 1, msg);
//     return makeResult(1, msg);
// }

QVariant gx_app_biometrics_load_token_ios(const QString &reason, QObject *ctx)
{
    // Return immediately; emit results asynchronously (like authenticate)
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        LAContext *la = [LAContext new];
        NSString *nsReason = reason.isEmpty() ? @"Unlock" : reason.toNSString();

        NSMutableDictionary *q = [baseKeychainQuery() mutableCopy];
        q[(__bridge id)kSecMatchLimit] = (__bridge id)kSecMatchLimitOne;
        q[(__bridge id)kSecReturnData] = @YES;
        q[(__bridge id)kSecUseAuthenticationContext] = la;
        // q[(__bridge id)kSecUseOperationPrompt] = nsReason;

        CFTypeRef item = nil;
        OSStatus st = SecItemCopyMatching((__bridge CFDictionaryRef)q, &item);

        if (st == errSecSuccess && item) {
            NSData *data = (__bridge_transfer NSData *)item;
            NSString *s = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            QString token = QString::fromNSString(s ?: @"");

            // emit back to Qt thread: authenticated + loginTokenReady
            QMetaObject::invokeMethod(ctx, [ctx, token]() {
                QMetaObject::invokeMethod(ctx, "authenticated", Qt::QueuedConnection,
                                          Q_ARG(int, 0), Q_ARG(QString, "ok"));
                QMetaObject::invokeMethod(ctx, "loginTokenReady", Qt::QueuedConnection,
                                          Q_ARG(int, 0), Q_ARG(QString, "ok"), Q_ARG(QString, token));
            }, Qt::QueuedConnection);

            return;
        }

        // error -> emit
        QString msg = QStringLiteral("Keychain read failed (%1)").arg((int)st);
        int code = 1;
        QMetaObject::invokeMethod(ctx, [ctx, code, msg]() {
            QMetaObject::invokeMethod(ctx, "authenticated", Qt::QueuedConnection,
                                      Q_ARG(int, code), Q_ARG(QString, msg));
            QMetaObject::invokeMethod(ctx, "loginTokenReady", Qt::QueuedConnection,
                                      Q_ARG(int, code), Q_ARG(QString, msg), Q_ARG(QString, QString()));
        }, Qt::QueuedConnection);
    });

    return QVariantMap{{"code", 0}, {"message", "started"}};
}


// Load token (prompts biometrics)
// extern "C" QVariant gx_app_biometrics_load_token_ios(const QString &reason, QObject *ctx)
// {
//     LAContext *la = [LAContext new];
//     NSString *nsReason = reason.isEmpty() ? @"Unlock" : reason.toNSString();

//     NSMutableDictionary *q = [baseKeychainQuery() mutableCopy];
//     q[(__bridge id)kSecMatchLimit] = (__bridge id)kSecMatchLimitOne;
//     q[(__bridge id)kSecReturnData] = @YES;
//     q[(__bridge id)kSecUseAuthenticationContext] = la;
//     q[(__bridge id)kSecUseOperationPrompt] = nsReason;

//     CFTypeRef item = nil;
//     OSStatus st = SecItemCopyMatching((__bridge CFDictionaryRef)q, &item);

//     if (st == errSecSuccess && item) {
//         NSData *data = (__bridge_transfer NSData *)item;
//         NSString *s = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
//         QString token = QString::fromNSString(s ?: @"");

//         QVariantMap res = makeResult(0, QStringLiteral("OK"), token);
//         // authenticated/loginTokenReady are emitted in C++ wrapper (like Android)
//         return res;
//     }

//     // Map common errors
//     QString msg;
//     int code = 1;
//     if (st == errSecUserCanceled) { code = 3; msg = QStringLiteral("User cancelled"); }
//     else if (st == errSecAuthFailed) { code = 2; msg = QStringLiteral("Authentication failed"); }
//     else if (st == errSecItemNotFound) { code = 7; msg = QStringLiteral("No token stored"); }
//     else { msg = QStringLiteral("Keychain read failed (%1)").arg((int)st); }

//     emitAuthenticated(ctx, code, msg);
//     return makeResult(code, msg, QString());
// }
