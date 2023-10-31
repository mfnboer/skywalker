// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;
import org.qtproject.qt.android.bindings.QtActivity;

import java.lang.String;

import android.util.Log;

public class SkywalkerActivity extends QtActivity {
    private static final String LOGTAG = "SkywalkerActivity";

    // Avoid the app to close when the user presses the back button.
    public void goToBack() {
        Log.d(LOGTAG, "Moving task to back");
        moveTaskToBack(true);
    }
}
