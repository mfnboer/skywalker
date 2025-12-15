// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.ScreenUtils;
import com.gmail.mfnboer.VirtualKeyboardListener;

import com.gmail.mfnboer.FileUtils;
import com.gmail.mfnboer.NewMessageChecker;
import com.gmail.mfnboer.NewMessageNotifier;
import com.gmail.mfnboer.VideoTranscoder;

import org.qtproject.qt.android.QtNative;
import org.qtproject.qt.android.bindings.QtActivity;

import java.lang.String;

import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import java.util.ArrayList;
import android.util.Log;

// For EdgeToEdge
import androidx.core.view.WindowCompat;

public class SkywalkerActivity extends QtActivity {
    private static final String LOGTAG = "SkywalkerActivity";
    private static final int MAX_TEXT_LEN = 32768;

    private static SkywalkerActivity sActivity = null;

    public static final String INTENT_ACTION_SHOW_NOTIFICATIONS = "com.gmail.mfnboer.skywalker.showNotifications";
    public static final String INTENT_ACTION_SHOW_DIRECT_MESSAGES = "com.gmail.mfnboer.skywalker.showDirectMessages";
    public static final String INTENT_EXTRA_DIRECT_MESSAGE = "com.gmail.mfnboer.skywalker.directMessage";

    public static native void emitSharedTextReceived(String text);
    public static native void emitSharedImageReceived(String uri, String text);
    public static native void emitSharedVideoReceived(String uri, String text);
    public static native void emitSharedDmTextReceived(String text);
    public static native void emitShowNotifications();
    public static native void emitShowDirectMessages();
    public static native void emitShowLink(String uri);

    private boolean mIsIntentPending = false;
    private boolean mIsReady = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        sActivity = this;

        // Enable EdgeToEdge mode, i.e. full screen.
        // WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
        WindowCompat.enableEdgeToEdge(getWindow());

        Log.d(LOGTAG, "onCreate");
        ScreenUtils.init(this);
        VirtualKeyboardListener.init(this);

        Intent intent = getIntent();
        if (intent == null)
            return;

        String action = intent.getAction();
        if (action == null)
            return;

        Log.d(LOGTAG, "Intent action: " + action + " type:" + intent.getType());

        // This should not be needed, but it looks like on some phones, the
        // intent is not kept.
        setIntent(intent);

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
        else if (action.equals(Intent.ACTION_SEND_MULTIPLE))
            handleActionSendMultiple(intent);
        else if (action.equals(INTENT_ACTION_SHOW_NOTIFICATIONS))
            handleActionShowNotifications(intent);
        else if (action.equals(INTENT_ACTION_SHOW_DIRECT_MESSAGES))
            handleActionShowDirectMessages(intent);
        else if (action.equals(Intent.ACTION_VIEW))
            handleActionShowLink(intent);
        else
            Log.d(LOGTAG, "Unsupported intent action: " + intent.getAction());
    }

    private void handleActionShowNotifications(Intent intent) {
        Log.d(LOGTAG, "Handle SHOW_NOTIFICATIONS");
        emitShowNotifications();
    }

    private void handleActionShowDirectMessages(Intent intent) {
        Log.d(LOGTAG, "Handle SHOW_DIRECT_MESSAGES");
        emitShowDirectMessages();
    }

    private void handleActionShowLink(Intent intent) {
        Log.d(LOGTAG, "Handle SHOW_LINK");

        Uri data = intent.getData();
        if (data == null) {
            Log.d(LOGTAG, "Empty data received");
            return;
        }
        // Android helpfully strips off the beginning of the link. Since the QML link code
        // expects the prefix, we just stick it back on here before sending it along.
        String path = "https://bsky.app" + data.getPath();
        Log.d(LOGTAG, "Handling the link: " + path);

        emitShowLink(path);
    }

    private String getMimeType(Intent intent) {
        String type = intent.getType();
        Log.d(LOGTAG, "Intent type: " + type);

        if (type != null && !type.startsWith("*"))
            return type;

        String text = intent.getStringExtra(Intent.EXTRA_TEXT);

        if (text != null)
            return "text/plain";

        Uri uri = (Uri)intent.getParcelableExtra(Intent.EXTRA_STREAM);
        return getContentResolver().getType(uri);
    }

    private void handleActionSend(Intent intent) {
        Log.d(LOGTAG, "Handle ACTION_SEND, DM: " + intent.getBooleanExtra(INTENT_EXTRA_DIRECT_MESSAGE, false));

        String type = getMimeType(intent);
        Log.d(LOGTAG, "Mime type: " + type);

        if (intent.getType().equals("text/plain")) {
            handleSharedText(intent);
            return;
        }

        if (intent.getType().startsWith("image/")) {
            handleSharedImage(intent);
            return;
        }

        if (intent.getType().startsWith("video/")) {
            handleSharedVideo(intent);
            return;
        }

        Log.d(LOGTAG, "Unsupported intent type: " + intent.getType());
    }

    private void handleActionSendMultiple(Intent intent) {
        ArrayList<Uri> uris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
        Log.d(LOGTAG, "Handle ACTION_SEND_MULTIPLE: " + uris.size());

        for (Uri uri : uris) {
            if (uri == null)
                continue;

            String mimeType = getContentResolver().getType(uri);
            Log.d(LOGTAG, "URI: " + uri + " mimetype: " + mimeType);

            if (mimeType.startsWith("image/"))
                emitSharedImageReceived(uri.toString(), "");
            else
                Log.w(LOGTAG, "Got non-image uri: " + uri + "mimetype: " + mimeType);
        }
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

        if (intent.getBooleanExtra(INTENT_EXTRA_DIRECT_MESSAGE, false))
            emitSharedDmTextReceived(sharedText);
        else
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

    private void handleSharedVideo(Intent intent) {
        Uri uri = (Uri)intent.getParcelableExtra(Intent.EXTRA_STREAM);

        if (uri == null) {
            Log.d(LOGTAG, "Empty video uri received");
            return;
        }

        String text = intent.getStringExtra(Intent.EXTRA_TEXT);

        if (text == null)
            text = "";

        if (text.length() > MAX_TEXT_LEN)
            text = text.substring(0, MAX_TEXT_LEN);

        String uriString = uri.toString();
        Log.d(LOGTAG, "Shared video: " + uriString);
        Log.d(LOGTAG, "Extra text  : " + text);
        emitSharedVideoReceived(uriString, text);
    }



    // Avoid the app to close when the user presses the back button.
    public void goToBack() {
        Log.d(LOGTAG, "Moving task to back");
        moveTaskToBack(true);
    }

    public void transcodeVideo(String inputFilePath, String outputFilePath, int height, int startMs, int endMs, boolean removeAudio) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                VideoTranscoder.transcodeVideo(inputFilePath, outputFilePath, height, startMs, endMs, removeAudio);
            }
        });
    }

    public static SkywalkerActivity getInstance() {
        return sActivity;
    }

    public void startContentChooser(Intent intent, String title) {
        startActivity(Intent.createChooser(intent, title));
    }
}
