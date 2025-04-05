// Copyright (C) 2025 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerApplication;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.util.Log;

import java.util.List;

public class Translate {
    private static final String LOGTAG = "Translate";
    private static final String GOOGLE_TRANSLATE_PACKAGE_NAME = "com.google.android.apps.translate";
    private static final String GOOGLE_TRANSLATE_TAP_ACTIVITY = "com.google.android.apps.translate.copydrop.gm3.TapToTranslateActivity";
    private static final String GOOGLE_TRANSLATE_MAIN_ACTIVITY = "com.google.android.apps.translate.TranslateActivity";

    public static boolean translate(String text) {
        Intent intent = new Intent()
            .setAction(Intent.ACTION_PROCESS_TEXT)
            .setType("text/plain")
            .putExtra(Intent.EXTRA_PROCESS_TEXT, text)
            .putExtra(Intent.EXTRA_PROCESS_TEXT_READONLY, true);

        ActivityInfo googleTranslate = getGoogleTranslate(intent);

        // It seems the Google Translate pacakge is not always visible. In that case
        // try known TAP and MAIN acitivities.
        if (googleTranslate != null)
            intent.setClassName(googleTranslate.packageName, googleTranslate.name);
        else
            intent.setClassName(GOOGLE_TRANSLATE_PACKAGE_NAME, GOOGLE_TRANSLATE_TAP_ACTIVITY);

        if (startTranslateActivity(intent))
            return true;

        intent = new Intent()
            .setAction(Intent.ACTION_SEND)
            .setType("text/plain")
            .putExtra(Intent.EXTRA_TEXT, text)
            .setClassName(GOOGLE_TRANSLATE_PACKAGE_NAME, GOOGLE_TRANSLATE_MAIN_ACTIVITY);

        return startTranslateActivity(intent);
    }

    private static ActivityInfo getGoogleTranslate(Intent intent) {
        SkywalkerActivity activity = SkywalkerActivity.getInstance();
        PackageManager pm = activity.getPackageManager();
        List<ResolveInfo> activities = pm.queryIntentActivities(intent, 0);

        for (ResolveInfo info : activities) {
            Log.d(LOGTAG, "Package: " + info.activityInfo.packageName + " Name: " + info.activityInfo.name);
            if (info.activityInfo.packageName.equals(GOOGLE_TRANSLATE_PACKAGE_NAME))
                return info.activityInfo;
        }

        Log.d(LOGTAG, "Google Translate activity not found");
        return null;
    }

    private static boolean startTranslateActivity(Intent intent) {
        SkywalkerActivity activity = SkywalkerActivity.getInstance();

        try {
            activity.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Log.d(LOGTAG, e.getMessage());
            return false;
        }

        return true;
    }
}
