// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.FileUtils;
import com.gmail.mfnboer.NewMessageChecker;
import com.gmail.mfnboer.NewMessageNotifier;

import org.qtproject.qt.android.QtNative;
import org.qtproject.qt.android.bindings.QtActivity;

import java.lang.String;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;

public class SkywalkerActivity extends QtActivity {
    private static final String LOGTAG = "SkywalkerActivity";
    private static final int MAX_TEXT_LEN = 512;
    public static final String INTENT_ACTION_SHOW_NOTIFICATIONS = "com.gmail.mfnboer.skywalker.showNotifications";

    public static native void emitSharedTextReceived(String text);
    public static native void emitSharedImageReceived(String uri, String text);
    public static native void emitShowNotifications();

    private boolean mIsIntentPending = false;
    private boolean mIsReady = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(LOGTAG, "onCreate");

        Window window = this.getWindow();
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        window.setStatusBarColor(Color.BLACK);

        Intent intent = getIntent();
        if (intent == null)
            return;

        String action = intent.getAction();
        if (action == null)
            return;

        Log.d(LOGTAG, "Intent action: " + action + " type:" + intent.getType());

        // App is starting up and not ready to receive intents.
        mIsIntentPending = true;
    }

    @Override
    public void onStart() {
        Log.d(LOGTAG, "onStart");
        super.onStart();
        NewMessageChecker.stopChecker();
        NewMessageNotifier.clearNotifications();
    }

    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Log.d(LOGTAG, "onNewIntent");

        if (intent == null)
            return;

        Log.d(LOGTAG, "Intent action: " + intent.getAction() + ", type: " + intent.getType());
        setIntent(intent);

        if (mIsReady)
            handleIntent();
        else
            mIsIntentPending = true;
    }

    public void handlePendingIntent() {
        Log.d(LOGTAG, "handlePendingIntent");
        mIsReady = true;

        if (mIsIntentPending) {
            mIsIntentPending = false;
            handleIntent();
        }
    }

    private void handleIntent() {
        Log.d(LOGTAG, "handleIntent");
        Intent intent = getIntent();
        String action = intent.getAction();

        if (action.equals(Intent.ACTION_SEND))
            handleActionSend(intent);
        else if (action.equals(INTENT_ACTION_SHOW_NOTIFICATIONS))
            handleActionShowNotifications(intent);
        else
            Log.d(LOGTAG, "Unsupported intent action: " + intent.getAction());
    }

    private void handleActionShowNotifications(Intent intent) {
        Log.d(LOGTAG, "Handle SHOW_NOTIFICATIONS");
        emitShowNotifications();
    }

    private void handleActionSend(Intent intent) {
        Log.d(LOGTAG, "Handle ACTION_SEND");

        if (intent.getType().equals("text/plain")) {
            handleSharedText(intent);
            return;
        }

        if (intent.getType().startsWith("image/")) {
            handleSharedImage(intent);
            return;
        }

        Log.d(LOGTAG, "Unsupported intent type: " + intent.getType());
    }

    private void handleSharedText(Intent intent) {
        String sharedText = intent.getStringExtra(Intent.EXTRA_TEXT);

        if (sharedText == null) {
            Log.d(LOGTAG, "Empty text received");
            return;
        }

        if (sharedText.length() > MAX_TEXT_LEN)
            sharedText = sharedText.substring(0, MAX_TEXT_LEN);

        Log.d(LOGTAG, "Shared text: " + sharedText);
        emitSharedTextReceived(sharedText);
    }

    private void handleSharedImage(Intent intent) {
        Uri uri = (Uri)intent.getParcelableExtra(Intent.EXTRA_STREAM);

        if (uri == null) {
            Log.d(LOGTAG, "Empty image uri received");
            return;
        }

        String text = intent.getStringExtra(Intent.EXTRA_TEXT);

        if (text == null)
            text = "";

        if (text.length() > MAX_TEXT_LEN)
            text = text.substring(0, MAX_TEXT_LEN);

        String uriString = uri.toString();
        Log.d(LOGTAG, "Shared image: " + uriString);
        Log.d(LOGTAG, "Extra text  : " + text);
        emitSharedImageReceived(uriString, text);
    }

    // Avoid the app to close when the user presses the back button.
    public void goToBack() {
        Log.d(LOGTAG, "Moving task to back");
        moveTaskToBack(true);
    }
}
