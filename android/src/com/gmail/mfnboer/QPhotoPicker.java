// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import com.gmail.mfnboer.FileUtils;

import java.lang.Math;
import java.lang.String;
import androidx.activity.result.contract.ActivityResultContracts.PickMultipleVisualMedia;
import androidx.activity.result.contract.ActivityResultContracts.PickVisualMedia;
import androidx.activity.result.contract.ActivityResultContracts.GetContent;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.PickVisualMediaRequest;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultCaller;
import androidx.appcompat.app.AppCompatActivity;
import android.content.ClipData;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.MediaStore;
import android.util.Log;

public class QPhotoPicker extends AppCompatActivity {
    private static final String LOGTAG = "QPhotoPicker";
    private static final int REQUEST_CODE = 42;

    private static boolean mPickVideo = false;
    private static int mMaxItems = 1;

    public static native void emitPhotoPicked(int fd, String mimeType, boolean last);
    public static native void emitPhotoPickCanceled();

    public static boolean isPhotoPickerAvailable() {
        boolean available = PickVisualMedia.isPhotoPickerAvailable();
        Log.d(LOGTAG, "Photo picker available: " + available);
        return available;
    }

    // Alternative is to put this in SkywalkerActivity itself to start the
    // activity from there without FLAG_ACTIVITY_NEW_TASK
    public static void start(boolean video, int maxItems) {
        Log.d(LOGTAG, "video: " + video + " maxItem: " + maxItems);
        int maxAllowedItems = MediaStore.getPickImagesMaxLimit();
        Log.d(LOGTAG, "Max allowed to pick: " + maxAllowedItems);
        mMaxItems = Math.min(maxItems, maxAllowedItems);
        mPickVideo = video;

        Context context = SkywalkerApplication.getContext();
        Intent intent = new Intent(context, QPhotoPicker.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(LOGTAG, "onCreate");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.VANILLA_ICE_CREAM && isPhotoPickerAvailable()) {
            if (mMaxItems == 1)
                pickVisualMedia();
            else
                pickMultipleVisualMedia();
        } else {
            getContent();
        }
    }

    // Before Android 15, this had a severe limitation, making almost all albums
    // inaccessible.
    public void pickVisualMedia() {
        Log.d(LOGTAG, "pickVisualMedia");

        ActivityResultLauncher<PickVisualMediaRequest> pickMedia =
                registerForActivityResult(new PickVisualMedia(), uri -> {
            if (uri != null) {
                selectUri(uri, true);
            }
            else {
                Log.d(LOGTAG, "No media selected");
                emitPhotoPickCanceled();
            }

            finish();
        });

        launchMediaPicker(pickMedia);
    }

    public void pickMultipleVisualMedia() {
        Log.d(LOGTAG, "pickMultipleVisualMedia");

        // Registers a photo picker activity launcher in single-select mode.
        ActivityResultLauncher<PickVisualMediaRequest> pickMedia =
                registerForActivityResult(new PickMultipleVisualMedia(mMaxItems), uris -> {
            Log.d(LOGTAG, "Items picked: " + uris.size());

            if (!uris.isEmpty()) {
                for (int i = 0; i < uris.size(); ++i) {
                    Uri uri = uris.get(i);
                    selectUri(uri, i == uris.size() - 1);
                }
            } else {
                Log.d(LOGTAG, "No media selected");
                emitPhotoPickCanceled();
            }

            finish();
        });

        launchMediaPicker(pickMedia);
    }

    private void launchMediaPicker(ActivityResultLauncher<PickVisualMediaRequest> pickMedia) {
        pickMedia.launch(new PickVisualMediaRequest.Builder()
            .setMediaType(mPickVideo ? PickVisualMedia.ImageAndVideo.INSTANCE : PickVisualMedia.ImageOnly.INSTANCE)
            .build());
    }

    public void getContent() {
        Log.d(LOGTAG, "getContent");
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");
        String[] mimeTypesWithVideo = {"image/*", "video/*"};
        String[] mimeTypesImageOnly = {"image/*"};

        if (mMaxItems > 1)
            intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);

        intent.putExtra(Intent.EXTRA_MIME_TYPES, mPickVideo ? mimeTypesWithVideo : mimeTypesImageOnly);
        intent.addCategory(Intent.CATEGORY_OPENABLE);

        startActivityForResult(Intent.createChooser(intent, "Select File"), REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_CODE && resultCode == RESULT_OK) {
            Uri uri = data.getData();
            ClipData clipData = data.getClipData();

            if (uri != null) {
                selectUri(uri, true);
            } else if (clipData != null) {
                int picked = clipData.getItemCount();
                Log.d(LOGTAG, "Items picked: " + picked);
                int selectCount = Math.min(picked, mMaxItems);

                for (int i = 0; i < selectCount; ++i) {
                    Uri clipUri = clipData.getItemAt(i).getUri();
                    selectUri(clipUri, i == selectCount - 1);
                }

            } else {
                Log.d(LOGTAG, "No media selected");
                emitPhotoPickCanceled();
            }
        }
        else
        {
            Log.d(LOGTAG, "requestCode: " + requestCode + " resultCode: " + resultCode);
            emitPhotoPickCanceled();
        }

        finish();
    }

    private void selectUri(Uri uri, boolean last) {
        Context context = SkywalkerApplication.getContext();
        String mimeType = context.getContentResolver().getType(uri);

        Log.d(LOGTAG, "Selected URI: " + uri + " mimetype: " + mimeType);
        int fd = FileUtils.openContentUri(uri);
        emitPhotoPicked(fd, mimeType, last);
    }
}
