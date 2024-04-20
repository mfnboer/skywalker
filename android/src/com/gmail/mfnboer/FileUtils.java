// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import java.io.File;
import java.io.FileNotFoundException;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.util.Log;

public class FileUtils {
    private static final String LOGTAG = "FileUtils";

    public static String resolveContentUriToFile(String uriString) {
        Context context = QtNative.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context to resolve content URI: " + uriString);
            return null;
        }

        Uri uri = Uri.parse(uriString);
        String[] projection = {MediaStore.Images.Media.DATA};
        Cursor cursor = context.getContentResolver().query(uri, projection, null, null, null);

        if (cursor != null && cursor.moveToFirst()) {
            int columnIndex = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
            String filePath = cursor.getString(columnIndex);
            cursor.close();
            return filePath;
        }

        return null;
    }

    public static int openContentUri(Uri contentUri) {
        Context context = QtNative.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context to open content: " + contentUri);
            return -1;
        }


        ContentResolver resolver = context.getContentResolver();
        ParcelFileDescriptor fileDescriptor;

        try {
            fileDescriptor = resolver.openFileDescriptor(contentUri, "r");
        } catch (FileNotFoundException e) {
            Log.w(LOGTAG, e.getMessage());
            return -1;
        }

        // detachFd will return a native file descriptor that we must close
        // later in C++ code
        int fd = fileDescriptor.detachFd();
        return fd;
    }

    public static int openContentUriString(String uriString) {
        Uri uri = Uri.parse(uriString);
        return openContentUri(uri);
    }

    public static String getPicturesPath() {
        File path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
        Log.d(LOGTAG, path.getAbsolutePath());
        return path.getAbsolutePath();
    }

    public static String getPicturesPath(String subDir) {
        File path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
        File subPath = new File(path, subDir);

        try {
            subPath.mkdirs();
        } catch (SecurityException e) {
            Log.w(LOGTAG, "Could not create path: " + subPath + " details: " + e.getMessage());
            return null;
        }

        Log.d(LOGTAG, subPath.getAbsolutePath());
        return subPath.getAbsolutePath();
    }

    public static String getAppDataPath(String subDir) {
        Context context = QtNative.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context to get app data path: " + subDir);
            return null;
        }

        File path = context.getFilesDir();
        File subPath = new File(path, subDir);

        try {
            subPath.mkdirs();
        } catch (SecurityException e) {
            Log.w(LOGTAG, "Could not create path: " + subPath + " details: " + e.getMessage());
            return null;
        }

        Log.d(LOGTAG, subPath.getAbsolutePath());
        return subPath.getAbsolutePath();
    }

// Make a media file show up in the gallery
public static void scanMediaFile(String fileName) {
    Log.d(LOGTAG, "Scan media file=" + fileName);

    MediaScannerConnection.scanFile(QtNative.getContext(),
            new String[]{ fileName }, null,
            new MediaScannerConnection.OnScanCompletedListener() {
                public void onScanCompleted(String path, Uri uri) {
                    Log.d(LOGTAG, "Scanned " + path + ":");
                    Log.d(LOGTAG, "  uri=" + uri);
                }
            }
        );
    }
}
