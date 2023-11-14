// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.database.Cursor;
import java.io.FileNotFoundException;
import android.provider.MediaStore;
import android.os.ParcelFileDescriptor;
import android.util.Log;

public class FileUtils {
    private static final String LOGTAG = "FileUtils";

    public static String resolveContentUriToFile(String uriString) {
        Context context = QtNative.getContext();
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
}
