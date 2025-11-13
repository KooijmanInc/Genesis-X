// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.cast;

public final class GXCastBridge {
    static {
        System.loadLibrary("hm-ui_arm64-v8a");
        System.loadLibrary("genesisx_arm64-v8a");
    }
    public static native void dispatchCommand(String cmd);

    public static native void onConnectionChanged(boolean connected, String deviceName);

    // public static native void onPlayerStatus(int playerState, long positionMs);

    public static native void onTrackFinished(String contentId);
}
