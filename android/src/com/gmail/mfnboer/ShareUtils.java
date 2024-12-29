// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerActivity;
import com.gmail.mfnboer.SkywalkerApplication;

import java.io.File;

import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import androidx.core.content.FileProvider;

public class ShareUtils {
    private static final String LOGTAG = "ShareUtils";

    public static void shareLink(String uriString, String subject) {
        Uri uri;
        try {
            uri = Uri.parse(uriString);
        } catch (Exception e) {
            Log.d(LOGTAG, "invalid uri");
            return;
        }

        if (uri == null) {
            Log.d(LOGTAG, "invalid uri");
            return;
        }

        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_SEND);
        intent.putExtra(Intent.EXTRA_TEXT, uriString);
        intent.setType("text/plain");
        SkywalkerActivity activity = SkywalkerActivity.getInstance();
        activity.startContentChooser(intent, "Share " + subject + " using:");
    }

    public static void shareMedia(String fileName) {
        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context to share media: " + fileName);
            return;
        }

        File file = new File(fileName);
        Uri uri;

        try {
            uri = FileProvider.getUriForFile(context, "com.gmail.mfnboer.skywalker.qtprovider", file);
        } catch (Exception e) {
            Log.w(LOGTAG, "invalid file: " + fileName);
            return;
        }

        if (uri == null) {
            Log.w(LOGTAG, "invalid uri for file: " + fileName);
            return;
        }

        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_SEND);
        intent.putExtra(Intent.EXTRA_STREAM, uri);
        intent.setClipData(ClipData.newRawUri(null, uri)); // show thumbnail on chooser
        intent.setType("image/jpg");
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        SkywalkerActivity activity = SkywalkerActivity.getInstance();
        activity.startContentChooser(intent, "Share image");
    }
}
