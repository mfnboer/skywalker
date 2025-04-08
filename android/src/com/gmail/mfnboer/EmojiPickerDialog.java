// Copyright (C) 2025 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerApplication;

import android.app.Activity;
import android.graphics.Color;
import android.util.Log;
import android.view.ViewGroup;
import android.view.Gravity;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;
import androidx.core.util.Consumer;
import androidx.emoji2.emojipicker.EmojiPickerView;
import androidx.emoji2.emojipicker.EmojiViewItem;

public class EmojiPickerDialog {
    private static final String LOGTAG = "QPhotoPicker";
    private static FrameLayout container = null;
    private static ViewGroup rootView = null;

    public static void show() {
        SkywalkerActivity activity = SkywalkerActivity.getInstance();

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                rootView = activity.findViewById(android.R.id.content);
                container = new FrameLayout(activity);
                FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT
                );
                params.gravity = Gravity.BOTTOM;
                container.setLayoutParams(params);
                container.setBackgroundColor(Color.WHITE);

                EmojiPickerView picker = new EmojiPickerView(activity);
                picker.setOnEmojiPickedListener(new Consumer<EmojiViewItem>() {
                    @Override
                    public void accept(@NonNull EmojiViewItem emoji) {
                        Log.d(LOGTAG, "Emoji picked: " + emoji.getEmoji());
                        rootView.removeView(container);
                        container = null;
                        rootView = null;
                    }
                });

                container.addView(picker);
                rootView.addView(container);
            }
        });
    }

    public static void dismiss() {
        if (container == null || rootView == null)
            return;

        SkywalkerActivity activity = SkywalkerActivity.getInstance();

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (rootView == null)
                    return;

                rootView.removeView(container);
                container = null;
                rootView = null;
            }
        });
    }
}
