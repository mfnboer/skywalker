// Copyright (C) 2025 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerApplication;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
//import android.graphics.Insets;
import android.os.Build;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class ScreenUtils {
    private static final String LOGTAG = "ScreenUtils";
    private static Activity sActivity;
    private static PowerManager.WakeLock sWakeLock = null;

    // Must match QEnums::InsertsSide
    public static final int INSETS_SIDE_TOP = 0;
    public static final int INSETS_SIDE_BOTTOM = 1;
    public static final int INSETS_SIDE_LEFT = 2;
    public static final int INSETS_SIDE_RIGHT = 3;

    public static void init(Activity activity) {
        sActivity = activity;
    }

    private static Insets getInsets(int insetType) {
        View rootView = ((ViewGroup)sActivity.findViewById(android.R.id.content)).getChildAt(0);
        WindowInsetsCompat insets = ViewCompat.getRootWindowInsets(rootView);
        return insets.getInsets(insetType);

        // if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
        //     Window window = sActivity.getWindow();
        //     WindowInsets insets = window.getDecorView().getRootWindowInsets();

        //     if (insets != null)
        //         return insets.getInsets(insetType);
        //     else
        //         Log.w(LOGTAG, "Cannot get window insets controller");
        // }

        // return null;
    }

    private static int getInsetsSide(Insets insets, int side) {
        if (insets == null)
            return 0;

        switch (side) {
            case INSETS_SIDE_TOP:
                return insets.top;
            case INSETS_SIDE_BOTTOM:
                return insets.bottom;
            case INSETS_SIDE_LEFT:
                return insets.left;
            case INSETS_SIDE_RIGHT:
                return insets.right;
            default:
                Log.w(LOGTAG, "Unknown side: " + side);
                return 0;
        }
    }

    public static int getNavigationBarSize(int side) {
        Insets insets = getInsets(WindowInsetsCompat.Type.navigationBars());
        return getInsetsSide(insets, side);
    }

    public static int getImeSize(int side) {
        Insets insets = getInsets(WindowInsetsCompat.Type.ime());
        return getInsetsSide(insets, side);
    }

    public static int getStatusBarSize(int side) {
        Insets insets = getInsets(WindowInsetsCompat.Type.statusBars());
        return getInsetsSide(insets, side);
    }

    public static void setStatusBarTransparent(boolean transparent, int color, boolean isLightMode) {
        Log.d(LOGTAG, "Set status bar transparent: " + transparent + " color: " + color + " light: " + isLightMode);
        sActivity.runOnUiThread(new StatusBarSetter(sActivity, transparent, color, isLightMode));
    }

    private static class StatusBarSetter implements Runnable {
        private Activity mActivity;
        private boolean mTransparent;
        private int mColor;
        private boolean mIsLightMode;

        StatusBarSetter(Activity activity, boolean transparent, int color, boolean isLightMode) {
            mActivity = activity;
            mTransparent = transparent;
            mColor = color;
            mIsLightMode = isLightMode;
        }

        @Override
        public void run() {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                Window window = mActivity.getWindow();
                window.setStatusBarColor(mTransparent ? Color.TRANSPARENT : mColor);
                window.setDecorFitsSystemWindows(!mTransparent);

                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                    WindowInsetsController insetsController = window.getInsetsController();

                    if (insetsController != null)
                        insetsController.setSystemBarsAppearance(mIsLightMode ? WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS : 0, WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS);
                    else
                        Log.w(LOGTAG, "Cannot get window insets controller");
                }
            }
        }
    }

    public static void setStatusBarColor(int color, boolean isLightMode) {
        Log.d(LOGTAG, "Set status bar color: " + color + " light: " + isLightMode);
        sActivity.runOnUiThread(new StatusBarColorSetter(sActivity, color, isLightMode));
    }

    private static class StatusBarColorSetter implements Runnable {
        private Activity mActivity;
        private int mColor;
        private boolean mIsLightMode;

        StatusBarColorSetter(Activity activity, int color, boolean isLightMode) {
            mActivity = activity;
            mColor = color;
            mIsLightMode = isLightMode;
        }

        @Override
        public void run() {
            Window window = mActivity.getWindow();
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.setStatusBarColor(mColor); // TODO: change when targetting Android 15

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                WindowInsetsController insetsController = window.getInsetsController();

                if (insetsController != null)
                    insetsController.setSystemBarsAppearance(mIsLightMode ? WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS : 0, WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS);
                else
                    Log.w(LOGTAG, "Cannot get window insets controller");
            }
        }
    }

    public static void setNavigationBarColor(int color, boolean isLightMode) {
        Log.d(LOGTAG, "Set navigation bar color: " + color + " light: " + isLightMode);
        sActivity.runOnUiThread(new NavigationBarColorSetter(sActivity, color, isLightMode));
    }

    private static class NavigationBarColorSetter implements Runnable {
        private Activity mActivity;
        private int mColor;
        private boolean mIsLightMode;

        NavigationBarColorSetter(Activity activity, int color, boolean isLightMode) {
            mActivity = activity;
            mColor = color;
            mIsLightMode = isLightMode;
        }

        @Override
        public void run() {
            Window window = mActivity.getWindow();
            window.setNavigationBarColor(mColor);

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                WindowInsetsController insetsController = window.getInsetsController();

                if (insetsController != null)
                    insetsController.setSystemBarsAppearance(mIsLightMode ? WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS : 0, WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS);
                else
                    Log.w(LOGTAG, "Cannot get window insets controller");
            }
        }
    }

    public static void setKeepScreenOn(boolean keepOn) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Window window = sActivity.getWindow();
                if (window == null) {
                    Log.w(LOGTAG, "Cannot get window");
                    return;
                }

                Context context = SkywalkerApplication.getContext();
                if (context == null) {
                    Log.w(LOGTAG, "No context");
                    return;
                }

                PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);

                if (keepOn) {
                    window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                    if (sWakeLock == null) {
                        sWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "SpiralFunTag");
                        sWakeLock.acquire();
                    }
                } else {
                    window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                    if (sWakeLock != null) {
                        sWakeLock.release();
                        sWakeLock = null;
                    }
                }
            }
        });
    }
}
