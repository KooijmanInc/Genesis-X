// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.photopicker;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;

import androidx.core.content.FileProvider;

import java.io.*;

public class GXPhotoPickerActivity extends Activity {

    private static final int REQ_CAMERA  = 31001;
    private static final int REQ_GALLERY = 31002;

    private File tempFile;

    private static native void onImageReady(String path);
    private static native void onError(String message);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String mode = getIntent().getStringExtra("mode");
        if ("camera".equals(mode)) {
            launchCamera();
        } else if ("gallery".equals(mode)) {
            launchGallery();
        } else {
            onError("Invalid mode");
            finish();
        }
    }

    private void launchCamera() {
        try {
            tempFile = new File(getCacheDir(), "photo.jpg");

            Uri uri = FileProvider.getUriForFile(
                    this,
                    getPackageName() + ".fileprovider",
                    tempFile
            );

            Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
            intent.putExtra(MediaStore.EXTRA_OUTPUT, uri);
            intent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

            startActivityForResult(intent, REQ_CAMERA);
        } catch (Exception e) {
            onError(e.toString());
            finish();
        }
    }

    private void launchGallery() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("image/*");
        startActivityForResult(intent, REQ_GALLERY);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode != RESULT_OK) {
            onError("Cancelled");
            finish();
            return;
        }

        try {
            if (requestCode == REQ_CAMERA) {
                if (tempFile != null && tempFile.exists()) {
                    onImageReady(tempFile.getAbsolutePath());
                } else {
                    onError("Camera file missing");
                }
                finish();
                return;
            }

            if (requestCode == REQ_GALLERY) {
                if (data == null || data.getData() == null) {
                    onError("No image selected");
                    finish();
                    return;
                }

                Uri uri = data.getData();
                File out = new File(getCacheDir(), "photo.jpg");
                copyUriToFile(uri, out);

                onImageReady(out.getAbsolutePath());
                finish();
            }
        } catch (Exception e) {
            onError(e.toString());
            finish();
        }
    }

    private void copyUriToFile(Uri uri, File out) throws Exception {
        try (InputStream in = getContentResolver().openInputStream(uri);
             OutputStream os = new FileOutputStream(out)) {

            byte[] buf = new byte[4096];
            int len;
            while ((len = in.read(buf)) > 0) os.write(buf, 0, len);
        }
    }
}
