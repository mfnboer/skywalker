// Copyright (C) 2025 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerApplication;

import android.app.Activity;
import android.graphics.Color;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.view.ViewGroup;
import android.view.Gravity;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;
import androidx.core.util.Consumer;
import androidx.emoji2.emojipicker.EmojiPickerView;
import androidx.emoji2.emojipicker.EmojiViewItem;

public class EmojiPickerDialog {
    private static final String LOGTAG = "EmojiPicker";
    private static FrameLayout container = null;
    private static ViewGroup rootView = null;

    public static native void emitEmojiPicked(String emoji);

    public static void show(boolean isLightMode) {
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
                container.setBackgroundColor(isLightMode ? Color.WHITE : Color.BLACK);

                ContextThemeWrapper pickerContext = new ContextThemeWrapper(
                    activity, isLightMode ? android.R.style.Theme_Light : android.R.style.Theme_Black);

                EmojiPickerView picker = new EmojiPickerView(pickerContext);
                picker.setEmojiGridColumns(7);
                picker.setOnEmojiPickedListener(new Consumer<EmojiViewItem>() {
                    @Override
                    public void accept(@NonNull EmojiViewItem emojiViewItem) {
                        String emoji = emojiViewItem.getEmoji();
                        Log.d(LOGTAG, "Emoji picked: " + emoji);
                        destroyPicker();
                        emitEmojiPicked(emoji);
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
                destroyPicker();
            }
        });
    }

    private static void destroyPicker() {
        if (rootView != null) {
            rootView.removeView(container);
            container = null;
            rootView = null;
        }
    }
}
