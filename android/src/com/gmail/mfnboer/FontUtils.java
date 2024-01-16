// Copyright (C) 2024 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.Configuration;
import android.util.Log;

public class FontUtils {
    private static final String LOGTAG = "FontUtils";

    public static float getFontScale() {
        Context context = QtNative.getContext();
        Configuration config = context.getResources().getConfiguration();
        Log.d(LOGTAG, "Font scale: " + config.fontScale);
        return config.fontScale;
    }
}
