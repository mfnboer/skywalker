// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import android.content.Intent;
import android.net.Uri;
import android.util.Log;

public class ShareUtils {
    private static final String LOGTAG = "ShareUtils";

    public static void shareLink(String uriString, String subject) {
        if (QtNative.activity() == null)
            return;

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
        QtNative.activity().startActivity(
                Intent.createChooser(intent, "Share " + subject + " using:"));
    }


}
