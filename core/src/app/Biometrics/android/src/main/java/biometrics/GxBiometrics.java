// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package biometrics;

import android.content.Context;
import android.content.SharedPreferences;

import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;

import android.app.Activity;
import android.os.Build;
import android.os.CancellationSignal;
import android.os.Handler;
import android.os.Looper;

import android.util.Base64;

import java.nio.charset.StandardCharsets;
import java.security.KeyStore;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;

import org.qtproject.qt.android.QtActivityUtils;

public final class GxBiometrics {
    private static final String PREFS = "gx_biometrics";
    private static final String PREF_CT = "token_ct";
    private static final String PREF_IV = "token_iv";
    private static final String KEY_ALIAS = "gx_login_token_aes_gcm";

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

    public static boolean hasLoginToken() {
        Activity a = QtActivityUtils.getActivity();
        if (a == null) return false;
        SharedPreferences p = a.getSharedPreferences(PREFS, Context.MODE_PRIVATE);

        return p.contains(PREF_CT) && p.contains(PREF_IV);
    }

    public static boolean clearLoginToken() {
        Activity a = QtActivityUtils.getActivity();
        if (a == null) return false;
        SharedPreferences p = a.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
        p.edit().remove(PREF_CT).remove(PREF_IV).apply();

        return true;
    }

    public static void storeLoginToken(long objPtr, String reason, String token) {
        Activity a = QtActivityUtils.getActivity();
        if (a == null) {
            notifyQt(objPtr, 5, "No activity");
            return;
        }
        if (Build.VERSION.SDK_INT < 28) {
            notifyQt(objPtr, 1, "BiometricPrompt requires Andriod 9+");
            return;
        }

        Handler main = new Handler(Looper.getMainLooper());
        main.post(() -> {
            try {
                Cipher cipher = createEncryptCipher();

                android.hardware.biometrics.BiometricPrompt bp = buildFrameworkPrompt(a, reason != null ? reason : "Enable biometric login");

                CancellationSignal cancel = new CancellationSignal();

                bp.authenticate(
                    new android.hardware.biometrics.BiometricPrompt.CryptoObject(cipher),
                    cancel,
                    a.getMainExecutor(),
                    new android.hardware.biometrics.BiometricPrompt.AuthenticationCallback() {
                        @Override
                        public void onAuthenticationSucceeded(android.hardware.biometrics.BiometricPrompt.AuthenticationResult result) {
                            try {
                                Cipher c = result.getCryptoObject() != null ? result.getCryptoObject().getCipher() : cipher;

                                byte[] ct = c.doFinal(token.getBytes(StandardCharsets.UTF_8));
                                byte[] iv = c.getIV();

                                String b64ct = Base64.encodeToString(ct, Base64.NO_WRAP);
                                String b64iv = Base64.encodeToString(iv, Base64.NO_WRAP);

                                SharedPreferences p = a.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
                                p.edit().putString(PREF_CT, b64ct).putString(PREF_IV, b64iv).apply();

                                notifyQt(objPtr, 0, "Token stored");
                            } catch (Throwable t) {
                                notifyQt(objPtr, 5, "Encrypt/store failed: " + t.getMessage());
                            }
                        }

                        @Override
                        public void onAuthenticationError(int errorCode, CharSequence errString) {
                            notifyQt(objPtr, mapFrameworkError(errorCode), errString != null ? errString.toString() : "error");
                        }

                        @Override
                        public void onAuthenticationFailed() {
                            notifyQt(objPtr, 3, "failed");
                        }
                    }
                );
            } catch (Throwable t) {
                notifyQt(objPtr, 5, "storeLoginToken error: " + t.getMessage());
            }
        });
    }

    public static void loadLoginToken(long objPtr, String reason) {
        Activity a = QtActivityUtils.getActivity();
        if (a == null) {
            notifyQtToken(objPtr, 5, "No activity", "");
            return;
        }
        if (Build.VERSION.SDK_INT < 28) {
            notifyQtToken(objPtr, 1, "BiometricPrompt requires Andriod 9+", "");
            return;
        }

        SharedPreferences p = a.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
        String b64ct = p.getString(PREF_CT, null);
        String b64iv = p.getString(PREF_IV, null);
        if (b64ct == null || b64iv == null) {
            notifyQtToken(objPtr, 1, "No token stored", "");
            return;
        }

        byte[] ct = Base64.decode(b64ct, Base64.NO_WRAP);
        byte[] iv = Base64.decode(b64iv, Base64.NO_WRAP);

        Handler main = new Handler(Looper.getMainLooper());
        main.post(() -> {
            try {
                Cipher cipher = createDecryptCipher(iv);

                android.hardware.biometrics.BiometricPrompt bp =
                    buildFrameworkPrompt(a, reason != null ? reason : "Unlock to sign in");

                CancellationSignal cancel = new CancellationSignal();

                bp.authenticate(
                    new android.hardware.biometrics.BiometricPrompt.CryptoObject(cipher),
                    cancel,
                    a.getMainExecutor(),
                    new android.hardware.biometrics.BiometricPrompt.AuthenticationCallback() {
                        @Override
                        public void onAuthenticationSucceeded(android.hardware.biometrics.BiometricPrompt.AuthenticationResult result) {
                            try {
                                Cipher c = result.getCryptoObject() != null ? result.getCryptoObject().getCipher() : cipher;

                                byte[] pt = c.doFinal(ct);
                                String token = new String(pt, StandardCharsets.UTF_8);

                                notifyQtToken(objPtr, 0, "ok", token);
                            } catch (Throwable t) {
                                notifyQtToken(objPtr, 5, "Decrypt failed: " + t.getMessage(), "");
                            }
                        }

                        @Override
                        public void onAuthenticationError(int errorCode, CharSequence errString) {
                            notifyQtToken(objPtr, mapFrameworkError(errorCode), errString != null ? errString.toString() : "error", "");
                        }

                        @Override
                        public void onAuthenticationFailed() {
                            notifyQtToken(objPtr, 3, "failed", "");
                        }
                    }
                );

            } catch (Throwable t) {
                notifyQtToken(objPtr, 5, "loadLoginToken error: " + t.getMessage(), "");
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

    private static native void notifyQtToken(long objPtr, int code, String message, String token);

    private static SecretKey getOrCreateSecretKey() throws Exception {
        KeyStore ks = KeyStore.getInstance("AndroidKeyStore");
        ks.load(null);

        KeyStore.Entry entry = ks.getEntry(KEY_ALIAS, null);
        if (entry instanceof KeyStore.SecretKeyEntry) {
            return ((KeyStore.SecretKeyEntry) entry).getSecretKey();
        }

        KeyGenerator kg = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore");

        KeyGenParameterSpec.Builder b = new KeyGenParameterSpec.Builder(
            KEY_ALIAS,
            KeyProperties.PURPOSE_ENCRYPT | KeyProperties.PURPOSE_DECRYPT
        )
            .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
            .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
            .setUserAuthenticationRequired(true)
            .setInvalidatedByBiometricEnrollment(true);

        if (Build.VERSION.SDK_INT >= 30) {
            b.setUserAuthenticationParameters(
                0,
                android.hardware.biometrics.BiometricManager.Authenticators.BIOMETRIC_STRONG
                    | android.hardware.biometrics.BiometricManager.Authenticators.DEVICE_CREDENTIAL
            );
        } else {
            b.setUserAuthenticationValidityDurationSeconds(-1);
        }

        kg.init(b.build());
        return kg.generateKey();
    }

    private static Cipher createEncryptCipher() throws Exception {
        Cipher c = Cipher.getInstance("AES/GCM/NoPadding");
        c.init(Cipher.ENCRYPT_MODE, getOrCreateSecretKey());

        return c;
    }

    private static Cipher createDecryptCipher(byte[] iv) throws Exception {
        Cipher c = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(128, iv);
        c.init(Cipher.DECRYPT_MODE, getOrCreateSecretKey(), spec);

        return c;
    }

    private static int mapFrameworkError(int errorCode) {
        if (errorCode == android.hardware.biometrics.BiometricPrompt.BIOMETRIC_ERROR_LOCKOUT
                || errorCode == android.hardware.biometrics.BiometricPrompt.BIOMETRIC_ERROR_LOCKOUT_PERMANENT) {
            return 2;
        }
        if (errorCode == android.hardware.biometrics.BiometricPrompt.BIOMETRIC_ERROR_CANCELED
                || errorCode == android.hardware.biometrics.BiometricPrompt.BIOMETRIC_ERROR_USER_CANCELED) {
            return 4;
        }
        return 5;
    }
}
