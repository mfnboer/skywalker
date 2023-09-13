// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import android.content.Context;
import android.net.Uri;
import android.database.Cursor;
import android.provider.MediaStore;

public class FileUtils {

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
}
