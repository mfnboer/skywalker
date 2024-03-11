// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.FileUtils;
import com.gmail.mfnboer.NewMessageChecker;

import org.qtproject.qt.android.QtNative;
import org.qtproject.qt.android.bindings.QtActivity;

import java.lang.String;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

public class SkywalkerActivity extends QtActivity {
    private static final String LOGTAG = "SkywalkerActivity";
    private static final int MAX_TEXT_LEN = 512;
    private boolean mIsIntentPending = false;
    private boolean mIsReady = false;

    public static native void emitSharedTextReceived(String text);
    public static native void emitSharedImageReceived(String uri, String text);
    public static native void emitPause();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(LOGTAG, "onCreate");

        Intent intent = getIntent();
        if (intent == null)
            return;

        String action = intent.getAction();
        if (action == null)
            return;

        String type = intent.getType();
        if (type == null)
            return;

        Log.d(LOGTAG, "action: " + action + ", type: " + type);

        // App is starting up and not ready to receive intents.
        mIsIntentPending = true;
    }

    @Override
    public void onPause() {
        Log.d(LOGTAG, "onPause");
        emitPause();
        NewMessageChecker.startChecker();
        super.onPause();
    }

    @Override
    public void onResume() {
        Log.d(LOGTAG, "onResume");
        NewMessageChecker.stopChecker();
        super.onResume();
    }

    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Log.d(LOGTAG, "onNewIntent");

        if (intent == null)
            return;

        Log.d(LOGTAG, "action: " + intent.getAction() + ", type: " + intent.getType());
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

        if (!intent.getAction().equals(Intent.ACTION_SEND)) {
            Log.d(LOGTAG, "Unsupported intent action: " + intent.getAction());
            return;
        }

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
