// Copyright (C) 2025 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.KeyboardHeightProvider;
import com.gmail.mfnboer.ScreenUtils;
import com.gmail.mfnboer.SkywalkerApplication;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.view.Window;

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

        rootView.getViewTreeObserver().addOnGlobalLayoutListener(
            new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    int navigationBarHeight = ScreenUtils.getNavigationBarSize(ScreenUtils.INSETS_SIDE_BOTTOM);
                    int imeHeight = ScreenUtils.getImeSize(ScreenUtils.INSETS_SIDE_BOTTOM);
                    int keyboardHeight = imeHeight > navigationBarHeight ? imeHeight - navigationBarHeight : 0;
                    Log.d(LOGTAG, "Keyboard height: " + keyboardHeight + " nav: " + navigationBarHeight + " ime: " + imeHeight);

                    emitKeyboardHeightChanged(keyboardHeight);
                }
            }
        );
    }

    private static void installLegacyKeyboardListener() {
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
