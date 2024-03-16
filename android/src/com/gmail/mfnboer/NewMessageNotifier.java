// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import com.gmail.mfnboer.skywalker.R;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.graphics.drawable.Icon;
import android.os.Build;
import android.util.Log;

public class NewMessageNotifier {
    private static final String LOGTAG = "NewMessageNotifier";
    private static final String CHANNEL_ID = "NotificationsChannel";
    private static final String CHANNEL_NAME = "Notifications";
    private static final int IMPORTANCE = NotificationManager.IMPORTANCE_DEFAULT;
    private static final String DESCRIPTION = "Notifications of replies and mentions";

    // Values must be the same as OffLineMessageChecker::IconType
    private static final int IC_CHAT = 0;
    private static final int IC_FOLLOW = 1;
    private static final int IC_LIKE = 2;
    private static final int IC_MENTION = 3;
    private static final int IC_REPOST = 4;

    private static int sNextNotificationId = 1;
    private static Context mContext;

    private static int iconTypeToResource(int iconType) {
        switch (iconType) {
            case IC_CHAT:
                return R.drawable.ic_chat;
            case IC_FOLLOW:
                return R.drawable.ic_follow;
            case IC_LIKE:
                return R.drawable.ic_like;
            case IC_MENTION:
                return R.drawable.ic_mention;
            case IC_REPOST:
                return R.drawable.ic_repost;
            default:
                Log.w(LOGTAG, "Unknown icon type: " + iconType);
                return R.drawable.ic_chat;
        }
    }

    public static void setContext(Context context) {
        mContext = context;
    }

    public static void createNotificationChannel() {
        // API 26+
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Log.d(LOGTAG, "Create notification channel: " + CHANNEL_ID + " name: " + CHANNEL_NAME);
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, CHANNEL_NAME, IMPORTANCE);
            channel.setDescription(DESCRIPTION);
            Context context = QtNative.getContext();
            NotificationManager notificationManager = context.getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }

    public static void createNotification(String title, String msg, long when, int iconType, byte[] avatar) {
        Log.d(LOGTAG, "Create notification: " + sNextNotificationId + " title: " + title + " msg: " + msg);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext, CHANNEL_ID)
                .setSmallIcon(iconTypeToResource(iconType))
                .setContentTitle(title)
                .setWhen(when)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setAutoCancel(true);

        if (msg.length() > 0)
            builder.setContentText(msg);

        if (avatar.length > 0) {
            Icon avatarIcon = Icon.createWithData(avatar, 0, avatar.length);
            builder.setLargeIcon(avatarIcon);
        }

        NotificationManagerCompat.from(mContext).notify(sNextNotificationId, builder.build());
        ++sNextNotificationId;
    }
}
