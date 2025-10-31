// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package biometrics;

import android.app.Activity;
import android.os.Build;
import android.os.CancellationSignal;
import android.os.Handler;
import android.os.Looper;

import org.qtproject.qt.android.QtActivityUtils;

public final class GxBiometrics {

    // 0 = OK, 1 = NotAvailable, 2 = TemporarilyUnavailable
    public static Integer getStatus() {
        Activity a = QtActivityUtils.getActivity();
        if (Build.VERSION.SDK_INT < 28) return 1; // framework prompt not available
        try {
            android.hardware.biometrics.BiometricPrompt bp =
                    buildFrameworkPrompt(a, "Check availability");
            return 0; // device has framework prompt support; real canAuthenticate is limited pre-30
        } catch (Throwable t) {
            return 1;
        }
    }

    public static void authenticate(long objPtr, String reason) {
        Activity a = QtActivityUtils.getActivity();
        if (Build.VERSION.SDK_INT < 28) {
            notifyQt(objPtr, 1, "BiometricPrompt requires Android 9+");
            return;
        }

        Handler main = new Handler(Looper.getMainLooper());
        main.post(() -> {
            try {
                android.hardware.biometrics.BiometricPrompt bp =
                        buildFrameworkPrompt(a, reason);

                CancellationSignal cancel = new CancellationSignal();
                bp.authenticate(cancel, a.getMainExecutor(),
                        new android.hardware.biometrics.BiometricPrompt.AuthenticationCallback() {
                            @Override
                            public void onAuthenticationSucceeded(
                                    android.hardware.biometrics.BiometricPrompt.AuthenticationResult result) {
                                notifyQt(objPtr, 0, "ok");
                            }
                            @Override
                            public void onAuthenticationError(int errorCode, CharSequence errString) {
                                // Map lockouts to TemporarilyUnavailable
                                if (errorCode == android.hardware.biometrics.BiometricPrompt.BIOMETRIC_ERROR_LOCKOUT
                                 || errorCode == android.hardware.biometrics.BiometricPrompt.BIOMETRIC_ERROR_LOCKOUT_PERMANENT) {
                                    notifyQt(objPtr, 2, errString != null ? errString.toString() : "lockout");
                                } else if (errorCode == android.hardware.biometrics.BiometricPrompt.BIOMETRIC_ERROR_CANCELED
                                        || errorCode == android.hardware.biometrics.BiometricPrompt.BIOMETRIC_ERROR_USER_CANCELED) {
                                    notifyQt(objPtr, 4, errString != null ? errString.toString() : "canceled");
                                } else {
                                    notifyQt(objPtr, 5, errString != null ? errString.toString() : "error");
                                }
                            }
                            @Override
                            public void onAuthenticationFailed() {
                                notifyQt(objPtr, 3, "failed");
                            }
                        });
            } catch (Throwable t) {
                notifyQt(objPtr, 5, "prompt error: " + t.getMessage());
            }
        });
    }

    // --- helpers ---

    private static android.hardware.biometrics.BiometricPrompt buildFrameworkPrompt(Activity a, String subtitle) {
        android.hardware.biometrics.BiometricPrompt.Builder b =
                new android.hardware.biometrics.BiometricPrompt.Builder(a)
                        .setTitle("Biometric login")
                        .setSubtitle(subtitle);

        // On API 30+ we can allow device credential fallback:
        if (Build.VERSION.SDK_INT >= 30) {
            b.setAllowedAuthenticators(
                    android.hardware.biometrics.BiometricManager.Authenticators.BIOMETRIC_STRONG
                    | android.hardware.biometrics.BiometricManager.Authenticators.DEVICE_CREDENTIAL
            );
        } else {
            // On API 28–29 we must provide a negative button text:
            b.setNegativeButton("Cancel", a.getMainExecutor(), (d, which) -> {});
        }
        return b.build();
    }

    private static native void notifyQt(long objPtr, int code, String message);
}


// package biometrics;

// import android.app.Activity;

// import androidx.annotation.NonNull;
// import androidx.biometric.BiometricManager;
// import androidx.biometric.BiometricPrompt;
// import androidx.core.content.ContextCompat;
// import androidx.fragment.app.FragmentActivity;

// import org.qtproject.qt.android.QtActivityUtils;

// import java.util.concurrent.Executor;

// public final class GxBiometrics {

//     // 0 = OK, 1 = NotAvailable, 2 = TemporarilyUnavailable
//     public static Integer getStatus() {
//         Activity a = QtActivityUtils.getActivity();
//         BiometricManager bm = BiometricManager.from(a);
//         int can = bm.canAuthenticate(BiometricManager.Authenticators.BIOMETRIC_STRONG);
//         switch (can) {
//             case BiometricManager.BIOMETRIC_SUCCESS: return 0;
//             case BiometricManager.BIOMETRIC_ERROR_NO_HARDWARE:
//             case BiometricManager.BIOMETRIC_ERROR_NONE_ENROLLED: return 1;
//             case BiometricManager.BIOMETRIC_ERROR_HW_UNAVAILABLE: return 2;
//             default: return 2;
//         }
//     }

//     // Called from C++: authenticate(ptr, reason)
//     public static void authenticate(long objPtr, String reason) {
//         Activity a = QtActivityUtils.getActivity();
//         FragmentActivity fa = (FragmentActivity) a; // requires QtAppCompatActivity in Manifest

//         Executor ex = ContextCompat.getMainExecutor(fa);
//         BiometricPrompt prompt = new BiometricPrompt(
//                 fa,
//                 ex,
//                 new BiometricPrompt.AuthenticationCallback() {
//                     @Override
//                     public void onAuthenticationSucceeded(
//                             @NonNull BiometricPrompt.AuthenticationResult result) {
//                         notifyQt(objPtr, 0, "ok");
//                     }

//                     @Override
//                     public void onAuthenticationError(int errorCode, @NonNull CharSequence err) {
//                         if (errorCode == BiometricPrompt.ERROR_LOCKOUT
//                                 || errorCode == BiometricPrompt.ERROR_LOCKOUT_PERMANENT) {
//                             notifyQt(objPtr, 2, err.toString());
//                         } else if (errorCode == BiometricPrompt.ERROR_CANCELED
//                                 || errorCode == BiometricPrompt.ERROR_USER_CANCELED) {
//                             notifyQt(objPtr, 4, err.toString());
//                         } else {
//                             notifyQt(objPtr, 5, err.toString());
//                         }
//                     }

//                     @Override
//                     public void onAuthenticationFailed() {
//                         notifyQt(objPtr, 3, "failed");
//                     }
//                 });

//         BiometricPrompt.PromptInfo info = new BiometricPrompt.PromptInfo.Builder()
//                 .setTitle("Biometric login")
//                 .setSubtitle(reason)
//                 .setAllowedAuthenticators(BiometricManager.Authenticators.BIOMETRIC_STRONG)
//                 .build();

//         prompt.authenticate(info);
//     }

//     private static native void notifyQt(long objPtr, int code, String message);
// }


// // package biometrics;

// // import android.app.Activity;
// // import android.os.Handler;
// // import android.os.Looper;

// // import androidx.annotation.NonNull;
// // import androidx.biometric.BiometricManager;
// // import androidx.biometric.BiometricPrompt;
// // import androidx.core.content.ContextCompat;
// // import androidx.fragment.app.FragmentActivity;

// // import org.qtproject.qt.android.QtNative;

// // import java.util.concurrent.Executor;

// // public final class GxBiometrics {
// //     private static native void nativeOnAuthResult(long ctrPtr, int code, String message);

// //     public static Integer getStatus() {
// //         Activity a = QtNative.activity();
// //         if (a == null) return 2;
// //         BiometricManager bm = BiometricManager.from(a);
// //         int can = bm.canAuthenticate(BiometricManager.Authenticator.BIOMETRIC_STRONG
// //                 | BiometricManager.Authenticators.DEVICE_CREDENTIAL);
// //         switch (can) {
// //             case BiometricsManager.BIOMETRIC_SUCCESS: return 0;
// //             case BiometricsManager.BIOMETRIC_ERROR_NO_HARDWARE:
// //             case BiometricsManager.BIOMETRIC_ERROR_NONE_ENROLLED: return 1;
// //             case BiometricsManager.BIOMETRIC_ERROR_HW_UNAVAILABLE:
// //             default: return 2;
// //         }
// //     }

// //     public static void authenticate(final long ctxPtr, final String reason) {
// //         final Activity a = QtNative.activity();
// //         if (a == null) {
// //             nativeOnAuthResult(ctxPtr, 2, "No activity available");
// //             return;
// //         }
// //         final Executor ex = ContextCompat.getMainExecutor(a);
// //         final BiometricPrompt.PromptInfo info = new BiometricPrompt.PromptInfo.Builder()
// //                 .setTitle(reason != null && !reason.isEmpty() ? reason : "Confirm it's you")
// //                 .setAllowedAuthenticators(BiometricsManager.Authenticators.BIOMETRICS_STRONG
// //                         | BiometricManager.Authenticators.DEVICE_CREDENTIAL)
// //                 .build();

// //         final BiometricPrompt prompt = new BiometricPrompt(a, ex, new BiometricPrompt.AuthenticationCallback() {
// //             @Override public void onAuthenticationSucceeded(@NonNull BiometricPrompt.AuthenticationResult result) {
// //                 nativeOnAuthResult(ctxPtr, 0, "OK");
// //             }
// //             @Override public void onAuthenticationError(int errorCode, @NonNull CharSequence errString) {
// //                 int code;
// //                 switch (errorCode) {
// //                     case BiometricPrompt.ERROR_CANCELED:
// //                     case BiometricPrompt.ERROR_USER_CANCELED:
// //                     case BiometricPrompt.ERROR_NEGATIVE_BUTTON:
// //                         code = 1;
// //                         break;
// //                     default:
// //                         code = 3;
// //                 }

// //                 if (errorCode == BiometricPrompt.ERROR_CANCELED
// //                  || errorCode == BiometricPrompt.ERROR_USER_CANCELED
// //                  || errorCode == BiometricPrompt.ERROR_NEGATIVE_BUTTON) {
// //                     code = 4;
// //                 } else if (errorCode == BiometricPrompt.ERROR_LOCKOUT
// //                         || errorCode == BiometricPrompt.ERROR_LOCKOUT_PERMENENT
// //                         || errorCode == BiometricPrompt.ERROR_HW_UNAVAILABLE) {
// //                     code = 2;
// //                 }
// //                 nativeOnAuthResult(ctxPtr, code, errString.toString());
// //             }
// //             @Override public void onAuthenticationFailed() {
// //                 nativeOnAuthResult(ctxPtr, 3, "Authentication failed");
// //             }
// //         });

// //         new Handler(Looper.getMainLooper()).post(() -> prompt.authenticate(info));
// //     }
// // }
