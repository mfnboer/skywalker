// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import com.gmail.mfnboer.skywalker.R;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Icon;
import android.os.Build;
import android.text.Html;
import android.text.Spanned;
import android.util.Log;

public class NewMessageNotifier {
    private static final String LOGTAG = "NewMessageNotifier";

    // Values must be the same as OffLineMessageChecker::IconType
    private static final int IC_CHAT = 0;
    private static final int IC_FOLLOW = 1;
    private static final int IC_LIKE = 2;
    private static final int IC_MENTION = 3;
    private static final int IC_REPOST = 4;

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

    public static void createNotificationChannel(String id, String name, String description) {
        // API 26+
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Log.d(LOGTAG, "Create notification channel: " + id + " name: " + name);
            NotificationChannel channel = new NotificationChannel(id, name, NotificationManager.IMPORTANCE_DEFAULT);
            channel.setDescription(description);
            Context context = QtNative.getContext();
            NotificationManager notificationManager = context.getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }

    public static void createNotification(String channelId, int notificationId, String title, String msg, long when, int iconType, byte[] avatar) {
        Log.d(LOGTAG, "Create notification: " + channelId + " id: " + notificationId + " title: " + title + " msg: " + msg);
        Intent intent = new Intent(com.gmail.mfnboer.SkywalkerActivity.INTENT_ACTION_SHOW_NOTIFICATIONS);
        intent.setComponent(new ComponentName("com.gmail.mfnboer.skywalker", "com.gmail.mfnboer.SkywalkerActivity"));
        PendingIntent pendingIntent = PendingIntent.getActivity(mContext, 0, intent, PendingIntent.FLAG_IMMUTABLE);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext, channelId)
                .setSmallIcon(iconTypeToResource(iconType))
                .setContentTitle(title)
                .setWhen(when)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setContentIntent(pendingIntent)
                .setAutoCancel(true);

        if (msg.length() > 0) {
            Spanned formattedMsg = Html.fromHtml(msg, Html.FROM_HTML_MODE_COMPACT);
            builder.setContentText(formattedMsg);
            builder.setStyle(new NotificationCompat.BigTextStyle().bigText(formattedMsg));
        }

        if (avatar.length > 0) {
            Icon avatarIcon = Icon.createWithData(avatar, 0, avatar.length);
            builder.setLargeIcon(avatarIcon);
        }

        NotificationManagerCompat.from(mContext).notify(notificationId, builder.build());
    }

    public static void clearNotifications() {
        Log.d(LOGTAG, "Clear notifications");
        Context context = QtNative.getContext();
        NotificationManagerCompat.from(context).cancelAll();
    }
}
