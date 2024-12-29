// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.NewMessageNotifier;
import com.gmail.mfnboer.SkywalkerApplication;

import androidx.annotation.NonNull;
import androidx.work.BackoffPolicy;
import androidx.work.Configuration;
import androidx.work.Constraints;
import androidx.work.ExistingPeriodicWorkPolicy;
import androidx.work.NetworkType;
import androidx.work.Worker;
import androidx.work.WorkerParameters;
import androidx.work.WorkManager;
import androidx.work.PeriodicWorkRequest;
import androidx.work.multiprocess.RemoteWorkManager;

import android.app.NotificationManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.util.Log;

import java.lang.System;
import java.util.concurrent.TimeUnit;

public class NewMessageChecker extends Worker {
    private static final String LOGTAG = "NewMessageChecker";
    private static final String SETTINGS_FILE_NAME = "/settings/Skywalker/Skywalker.conf";

    static {
        try {
            // Load the QtCore lib first to initialize JNI
            Log.d(LOGTAG, "Load Qt6Core_arm64-v8a");
            System.loadLibrary("Qt6Core_arm64-v8a");
            Log.d(LOGTAG, "Loaded Qt6Core_arm64-v8a");
            Log.d(LOGTAG, "Load appskywalker_arm64-v8a");
            System.loadLibrary("appskywalker_arm64-v8a");
            Log.d(LOGTAG, "Loaded appskywalker_arm64-v8a");
        } catch (UnsatisfiedLinkError ule) {
            try {
                Log.d(LOGTAG, "Load Qt6Core_armeabi-v7a");
                System.loadLibrary("Qt6Core_armeabi-v7a");
                Log.d(LOGTAG, "Loaded Qt6Core_armeabi-v7a");
                Log.d(LOGTAG, "Load appskywalker_armeabi-v7a");
                System.loadLibrary("appskywalker_armeabi-v7a");
                Log.d(LOGTAG, "Loaded appskywalker_armeabi-v7a");
            } catch (UnsatisfiedLinkError ule7a) {
                Log.w(LOGTAG, "Cannot link appskywalker_armeabi-v7a lib: " + ule7a.getMessage());
            } catch (Exception e) {
                Log.w(LOGTAG, "Cannot load appskywalker_armeabi-v7a lib: " + e.getMessage());
            }
        } catch (Exception e) {
            Log.w(LOGTAG, "Cannot load appskywalker_arm64-v8a lib: " + e.getMessage());
        }
    }

    public static native int checkNewMessages(String settingsFileName, String libDir);

    private Context mContext;

    public NewMessageChecker(
        @NonNull Context context,
        @NonNull WorkerParameters params) {
        super(context, params);
        mContext = context;
    }

    @Override
    public Result doWork() {
        if (isStopped()) {
            Log.d(LOGTAG, "Worker is stopped");
            return Result.success();
        }

        Log.d(LOGTAG, "Check for new messages");
        Log.d(LOGTAG, "data dir: " + mContext.getDataDir().getPath());

        ApplicationInfo appInfo = mContext.getApplicationInfo();
        Log.d(LOGTAG, "native lib dir: " + appInfo.nativeLibraryDir);
        Log.d(LOGTAG, "source dir:" + appInfo.sourceDir);

        NewMessageNotifier.setContext(mContext);
        int exitCode = checkNewMessages(getSettingsFileName(), appInfo.nativeLibraryDir);

        if (exitCode == -1)
            return Result.retry();

        return Result.success();
    }

    private String getSettingsFileName() {
        String appDir = mContext.getFilesDir().getPath();
        return appDir + SETTINGS_FILE_NAME;
    }

    public static RemoteWorkManager getRemoteWorkManager(Context context) {
        return RemoteWorkManager.getInstance(context);
    }

    public static void startChecker(boolean wifiOnly) {
        startChecker(1, wifiOnly);
        startChecker(2, wifiOnly);
    }

    public static void startChecker(int id, boolean wifiOnly) {
        String taskName = getTaskName(id);
        Log.d(LOGTAG, "Start checker: " + taskName + " wifiOnly: " + wifiOnly);

        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context. Cannot start message checker");
            return;
        }

        NotificationManager notificationManager = context.getSystemService(NotificationManager.class);

        if (!notificationManager.areNotificationsEnabled()) {
            Log.d(LOGTAG, "Notifications are not enabled.");
            return;
        }

        Constraints constraints = new Constraints.Builder()
            .setRequiresBatteryNotLow(true)
            .setRequiredNetworkType(wifiOnly ? NetworkType.UNMETERED : NetworkType.CONNECTED)
            .build();

        PeriodicWorkRequest request = new PeriodicWorkRequest.Builder(
            NewMessageChecker.class, 15, TimeUnit.MINUTES)
                .setInitialDelay(1, TimeUnit.MINUTES)
                .setBackoffCriteria(BackoffPolicy.LINEAR, 450, TimeUnit.SECONDS)
                .setConstraints(constraints)
                .build();

        getRemoteWorkManager(context).enqueueUniquePeriodicWork(
            taskName,
            ExistingPeriodicWorkPolicy.REPLACE,
            request);
    }

    public static void stopChecker() {
        stopChecker(1);
        stopChecker(2);
    }

    public static void stopChecker(int id) {
        String taskName = getTaskName(id);
        Log.d(LOGTAG, "Stop checker: " + taskName);

        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context. Cannot stop message checker");
            return;
        }

        getRemoteWorkManager(context).cancelUniqueWork(taskName);
    }

    public static String getTaskName(int id) {
        return "checkNewMessages_" + id;
    }
}
