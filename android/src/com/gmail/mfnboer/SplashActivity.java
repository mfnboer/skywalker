// Copyright (C) 2026 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerActivity;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.util.Log;
import android.content.Intent;
import android.view.ViewTreeObserver;

public class SplashActivity extends Activity {

    private static final String TAG = "SplashActivity";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Set up an OnPreDrawListener to the root view.
        final View content = findViewById(android.R.id.content);
                content.getViewTreeObserver().addOnPreDrawListener(
                        new ViewTreeObserver.OnPreDrawListener() {
            @Override
            public boolean onPreDraw() {
                Intent intent = new Intent(SplashActivity.this, SkywalkerActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                startActivity(intent);

                content.getViewTreeObserver().removeOnPreDrawListener(this);
                finish();
                return true;
            }
        });
    }
}
