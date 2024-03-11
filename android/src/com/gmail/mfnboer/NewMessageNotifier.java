// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.R;

public class NewMessageNotifier {
    private static final String LOGTAG = "NewMessageNotifier";
    private static final String CHANNEL_ID = "NotificationsChannel";
    private static final String CHANNEL_NAME = "Notifications";
    private static final int IMPORTANCE = NotificationManager.IMPORTANCE_DEFAULT;
    private static final String DESCRIPTION = "Notifications of replies and mentions";
    private static int sNextNotificationId = 1;

    public static void createNotificationChannel() {
        // API 26+
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, CHANNEL_NAME, IMPORTANCE);
            channel.setDescription(DESCRIPTION);
            Context context = QtNative.getContext();
            NotificationManager notificationManager = context.getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }

    public static void createNotification(String title, String msg) {
        Context context = QtNative.getContext();
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context, CHANNEL_ID)
                .setSmallIcon(R.drawable.ic_dialog_alert)
                .setContentTitle(title)
                .setContentText(msg)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setAutoCancel(true);

        NotificationManagerCompat.from(context).notify(sNextNotificationId, builder.build());
        ++sNextNotificationId;
    }
}
