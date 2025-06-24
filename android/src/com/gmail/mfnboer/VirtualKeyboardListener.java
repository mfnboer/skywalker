// Copyright (C) 2025 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.KeyboardHeightProvider;
import com.gmail.mfnboer.SkywalkerApplication;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.Window;

import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class VirtualKeyboardListener {
    private static final String LOGTAG = "VirtualKeyboardListener";
    private static Activity sActivity;
    private static KeyboardHeightProvider keyboardHeightProvider;

    public static native void emitKeyboardHeightChanged(int height);

    public static void init(Activity activity) {
        sActivity = activity;
    }

    public static void installKeyboardListener()
    {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            installLegacyKeyboardListener();
            return;
        }

        final View rootView = ((ViewGroup)sActivity.findViewById(android.R.id.content)).getChildAt(0);

        ViewCompat.setOnApplyWindowInsetsListener(rootView, (v, insets) -> {
            int keyboardHeight = insets.getInsets(WindowInsetsCompat.Type.ime()).bottom;
            Log.d(LOGTAG, "Keyboard height: " + keyboardHeight);

            emitKeyboardHeightChanged(keyboardHeight);
            return insets;
        });
    }

    private static void installLegacyKeyboardListener() {
        Log.d(LOGTAG, "Install legacy keyboard listener");
        final View rootView = ((ViewGroup)sActivity.findViewById(android.R.id.content)).getChildAt(0);

        Context context = SkywalkerApplication.getContext();
        WindowManager wm = sActivity.getWindowManager();
        keyboardHeightProvider = new KeyboardHeightProvider(context, wm, rootView,
            new KeyboardHeightProvider.KeyboardHeightListener() {
                @Override
                public void onKeyboardHeightChanged(int keyboardHeight) {
                    emitKeyboardHeightChanged(keyboardHeight);
                }
            });
    }
}
