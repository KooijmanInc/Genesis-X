// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.photopicker;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;

public final class GXPhotoPicker {

    public static void openCamera(Context context) {
        Activity a = (context instanceof Activity) ? (Activity)context : null;
        if (a == null) return;

        Intent i = new Intent(a, GXPhotoPickerActivity.class);
        i.putExtra("mode", "camera");
        a.startActivity(i);
    }

    public static void openGallery(Context context) {
        Activity a = (context instanceof Activity) ? (Activity)context : null;
        if (a == null) return;

        Intent i = new Intent(a, GXPhotoPickerActivity.class);
        i.putExtra("mode", "gallery");
        a.startActivity(i);
    }
}
