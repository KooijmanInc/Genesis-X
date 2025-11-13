package com.genesisx.cast;

import android.content.Context;
import com.google.android.gms.cast.framework.CastOptions;
import com.google.android.gms.cast.framework.OptionsProvider;
import com.google.android.gms.cast.framework.SessionProvider;

import java.util.List;

public class GXCastOptionsProvider implements OptionsProvider {
    @Override public CastOptions getCastOptions(Context context) {
        final String DEFAULT_RECEIVER_APP_ID = "E6963F59";
        // final String DEFAULT_RECEIVER_APP_ID = "CC1AD845";
        return new CastOptions.Builder()
            .setReceiverApplicationId(DEFAULT_RECEIVER_APP_ID)
            .build();
    }

    @Override public List<SessionProvider> getAdditionalSessionProviders(Context context) {
        return null;
    }
}
