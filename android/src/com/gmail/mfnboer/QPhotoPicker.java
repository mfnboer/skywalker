// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import com.gmail.mfnboer.FileUtils;

import java.lang.String;
//import androidx.activity.result.contract.ActivityResultContracts.PickVisualMedia;
import androidx.activity.result.contract.ActivityResultContracts.GetContent;
import androidx.activity.result.ActivityResultLauncher;
//import androidx.activity.result.PickVisualMediaRequest;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultCaller;
import androidx.appcompat.app.AppCompatActivity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

public class QPhotoPicker extends AppCompatActivity {
    private static final String LOGTAG = "QPhotoPicker";
    private static final int REQUEST_CODE = 42;

    private static boolean pickVideo = false;

    public static native void emitPhotoPicked(int fd, String mimeType);
    public static native void emitPhotoPickCanceled();

    public static void start(boolean video) {
        pickVideo = video;
        Context context = QtNative.getContext();
        Intent intent = new Intent(context, QPhotoPicker.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(LOGTAG, "onCreate");

        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");
        String[] mimeTypesWithVideo = {"image/*", "video/*"};
        String[] mimeTypesImageOnly = {"image/*"};
        intent.putExtra(Intent.EXTRA_MIME_TYPES, pickVideo ? mimeTypesWithVideo : mimeTypesImageOnly);
        intent.addCategory(Intent.CATEGORY_OPENABLE);

        startActivityForResult(Intent.createChooser(intent, "Select File"), REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_CODE && resultCode == RESULT_OK) {
            Uri uri = data.getData();

            if (uri != null) {
                Context context = QtNative.getContext();
                String mimeType = context.getContentResolver().getType(uri);
                Log.d("PhotoPicker", "Selected URI: " + uri + " mimetype: " + mimeType);
                int fd = FileUtils.openContentUri(uri);
                emitPhotoPicked(fd, mimeType);
            } else {
                Log.d("PhotoPicker", "No media selected");
                emitPhotoPickCanceled();
            }
        }
        else
        {
            Log.d("PhotoPicker", "requestCode: " + requestCode + " resultCode: " + resultCode);
            emitPhotoPickCanceled();
        }

        finish();
    }

    // Code before adding video. With the launcher GetContent only takes a single mime tyoe
    //
    // public void onCreate(Bundle savedInstanceState) {
    //     super.onCreate(savedInstanceState);
    //     Log.d(LOGTAG, "onCreate");

    //     ActivityResultLauncher<String> getContent = registerForActivityResult(new GetContent(),
    //         new ActivityResultCallback<Uri>() {
    //             @Override
    //             public void onActivityResult(Uri uri) {
    //                 if (uri != null) {
    //                     Context context = QtNative.getContext();
    //                     String mimeType = context.getContentResolver().getType(uri);
    //                     Log.d("PhotoPicker", "Selected URI: " + uri + " mimetype: " + mimeType);
    //                     int fd = FileUtils.openContentUri(uri);
    //                     emitPhotoPicked(fd);
    //                 } else {
    //                     Log.d("PhotoPicker", "No media selected");
    //                     emitPhotoPickCanceled();
    //                 }

    //                 finish();
    //             }
    //     });

    //     getContent.launch("image/*");
    // }

    // PickVisualMediaRequest should be the way to go according to the Android documentation.
    // However, this has a serious permission limits which makes almost all photo albums
    // inaccessible to the user. Maybe it will be better in Android 14. Till then we
    // use GetContent instead.
    //
    // public void onCreate(Bundle savedInstanceState) {
    //     super.onCreate(savedInstanceState);
    //     Log.d(LOGTAG, "onCreate");

    //     // Registers a photo picker activity launcher in single-select mode.
    //     ActivityResultLauncher<PickVisualMediaRequest> pickMedia =
    //             registerForActivityResult(new PickVisualMedia(), uri -> {
    //         // Callback is invoked after the user selects a media item or closes the
    //         // photo picker.
    //         if (uri != null) {
    //             Log.d("PhotoPicker", "Selected URI: " + uri);
    //             int fd = FileUtils.openContentUri(uri);
    //             emitPhotoPicked(fd);
    //         } else {
    //             Log.d("PhotoPicker", "No media selected");
    //             emitPhotoPickCanceled();
    //         }

    //         finish();
    //     });

    //     // Launch the photo picker and let the user choose images and videos.
    //     pickMedia.launch(new PickVisualMediaRequest.Builder()
    //             .setMediaType(PickVisualMedia.ImageOnly.INSTANCE)
    //             .build());
    // }
}
