// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.cast;

import android.os.Bundle;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.widget.FrameLayout;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.mediarouter.app.MediaRouteButton;
import androidx.mediarouter.app.MediaRouteChooserDialog;
import androidx.mediarouter.media.MediaRouteSelector;
import com.google.android.gms.cast.CastMediaControlIntent;

import com.google.android.gms.cast.framework.CastContext;

public class GXCastChooserActivity extends AppCompatActivity {

    private MediaRouteButton routeButton;

    private static final String APP_ID = CastMediaControlIntent.DEFAULT_MEDIA_RECEIVER_APPLICATION_ID;
    // private static final String APP_ID = "E6963F59";

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String appId = CastContext.getSharedInstance(getApplicationContext())
                                      .getCastOptions()
                                      .getReceiverApplicationId();

        Log.d("GXCastOptionsProvider", "Using AppID = " + appId);

        MediaRouteSelector selector = new MediaRouteSelector.Builder()
                .addControlCategory(CastMediaControlIntent.categoryForCast(appId))
                .build();

        MediaRouteChooserDialog dlg = new MediaRouteChooserDialog(this);
        dlg.setRouteSelector(selector);
        dlg.setOnDismissListener(di -> finish());
        dlg.show();
        Log.d(APP_ID, "we got an app id");
    }

    // @Override protected void onCreate(Bundle savedInstanceState) {
    //     super.onCreate(savedInstanceState);

    //     // Themed context to avoid translucent background issues
    //     ContextThemeWrapper themed = new ContextThemeWrapper(
    //             this, androidx.appcompat.R.style.Theme_AppCompat_Light_NoActionBar);

    //     // Build a tiny layout with a MediaRouteButton and click it
    //     FrameLayout root = new FrameLayout(themed);
    //     setContentView(root);

    //     routeButton = new MediaRouteButton(themed);
    //     routeButton.setRouteSelector(
    //             CastContext.getSharedInstance(getApplicationContext()).getMergedSelector());

    //     root.addView(routeButton, new FrameLayout.LayoutParams(1, 1));

    //     // Show the official Cast route chooser dialog
    //     routeButton.post(routeButton::performClick);

    //     // Build selector *here* (no GXCastRouter helper needed)
    //     MediaRouteSelector selector = new MediaRouteSelector.Builder()
    //             .addControlCategory(CastMediaControlIntent.categoryForCast(APP_ID))
    //             .build();

    //     MediaRouteChooserDialog dlg = new MediaRouteChooserDialog(this);
    //     dlg.setRouteSelector(selector);
    //     dlg.setOnDismissListener(di -> finish()); // ← critical: return to Qt activity
    //     dlg.show();
    // }

    // @Override
    // protected void onCreate(@Nullable Bundle savedInstanceState) {
    //     super.onCreate(savedInstanceState);

    //     MediaRouteSelector selector = GXCastRouter.selector(this); // your helper that builds the selector

    //     MediaRouteChooserDialog dlg = new MediaRouteChooserDialog(this);
    //     dlg.setRouteSelector(selector);
    //     dlg.setOnDismissListener(di -> finish());   // <- critical: return to Qt activity
    //     dlg.show();

    //     // Themed context to avoid translucent background issues
    //     // ContextThemeWrapper themed = new ContextThemeWrapper(
    //     //         this, androidx.appcompat.R.style.Theme_AppCompat_Light_NoActionBar);

    //     // // Build a tiny layout with a MediaRouteButton and click it
    //     // FrameLayout root = new FrameLayout(themed);
    //     // setContentView(root);

        // routeButton = new MediaRouteButton(themed);
        // routeButton.setRouteSelector(
        //         CastContext.getSharedInstance(getApplicationContext()).getMergedSelector());

        // root.addView(routeButton, new FrameLayout.LayoutParams(1, 1));

        // // Show the official Cast route chooser dialog
        // routeButton.post(routeButton::performClick);
    // }

    @Override
    protected void onPause() {
        super.onPause();
        // When chooser takes over (or user cancels), close this bridge activity
        if (!isFinishing()) finish();
    }
}
