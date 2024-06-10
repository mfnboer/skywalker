// Copyright (C) 2024 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import com.gmail.mfnboer.SkywalkerActivity;

import androidx.appcompat.app.AppCompatActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class DirectMessageActivity extends AppCompatActivity {
    private static final String LOGTAG = "DirectMessageActivity";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(LOGTAG, "onCreate");
        handleIntent(getIntent());
    }

    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Log.d(LOGTAG, "onNewIntent");
        handleIntent(intent);
    }

    private void handleIntent(Intent intent) {
        intent.setClass(this, SkywalkerActivity.class);
        intent.setIdentifier(SkywalkerActivity.INTENT_DIRECT_MESSAGE_ID);
        startActivity(intent);
    }
}
