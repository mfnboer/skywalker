// Copyright (C) 2025 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.ScreenUtils;

import android.content.Context;
import android.graphics.drawable.ColorDrawable;
import android.graphics.Rect;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.PopupWindow;

// Needed for API level 28, 29
// ScreenUtils.getImeSize only works from API level 30
public class KeyboardHeightProvider extends PopupWindow {
    private static final String LOGTAG = "KeyboardHeightProvider";

    public KeyboardHeightProvider(Context context, WindowManager windowManager, View parentView, KeyboardHeightListener listener) {
        super(context);

        LinearLayout popupView = new LinearLayout(context);
        popupView.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        popupView.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
            DisplayMetrics metrics = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(metrics);

            Rect rect = new Rect();
            popupView.getWindowVisibleDisplayFrame(rect);

            int keyboardHeight = metrics.heightPixels - (rect.bottom - rect.top);
            int resourceID = context.getResources().getIdentifier("status_bar_height", "dimen", "android");

            if (resourceID > 0) {
                keyboardHeight -= context.getResources().getDimensionPixelSize(resourceID);
            }

            if (keyboardHeight < 100) {
                keyboardHeight = 0;
            } else {
                keyboardHeight += ScreenUtils.getNavigationBarSize(ScreenUtils.INSETS_SIDE_BOTTOM);
            }

            Log.d(LOGTAG, "Keyboard height: " + keyboardHeight);

            if (listener != null) {
                listener.onKeyboardHeightChanged(keyboardHeight);
            }
        });

        setContentView(popupView);

        setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
        setInputMethodMode(PopupWindow.INPUT_METHOD_NEEDED);
        setWidth(0);
        setHeight(ViewGroup.LayoutParams.MATCH_PARENT);
        setBackgroundDrawable(new ColorDrawable(0));

        parentView.post(() -> showAtLocation(parentView, Gravity.NO_GRAVITY, 0, 0));
    }

    public interface KeyboardHeightListener {
        void onKeyboardHeightChanged(int keyboardHeight);
    }
}
