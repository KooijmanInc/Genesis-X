// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package org.qtproject.qt.android;

import android.app.Activity;

public final class QtActivityUtils {
    private QtActivityUtils() {}
    public static Activity getActivity() { return QtNative.activity(); }
}
