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
import androidx.work.PeriodicWorkRequest;
import androidx.work.multiprocess.RemoteWorkManager;

import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.app.NotificationManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.util.Log;

import java.lang.System;
import java.util.concurrent.TimeUnit;

public class NewMessageChecker extends Worker {
    private static final String LOGTAG = "NewMessageChecker";
    private static final String SETTINGS_FILE_NAME = "/settings/Skywalker/Skywalker.conf";
    private static final int EXIT_RETRY = -1;
    private static final int EXIT_FAILED = -2;

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

    public native int checkNewMessages(String settingsFileName, String libDir);

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
            return Result.failure();
        }

        Log.d(LOGTAG, "Check for new messages");

        if (isMemoryLow()) {
            Log.w(LOGTAG, "Memory is low, postpone checking");
            return Result.retry();
        }

        Log.d(LOGTAG, "data dir: " + mContext.getDataDir().getPath());

        ApplicationInfo appInfo = mContext.getApplicationInfo();
        Log.d(LOGTAG, "native lib dir: " + appInfo.nativeLibraryDir);
        Log.d(LOGTAG, "source dir:" + appInfo.sourceDir);

        NewMessageNotifier.setContext(mContext);
        int exitCode = checkNewMessages(getSettingsFileName(), appInfo.nativeLibraryDir);

        if (exitCode == EXIT_RETRY)
            return Result.retry();
        else if (exitCode == EXIT_FAILED)
            return Result.failure();

        return Result.success();
    }

    private String getSettingsFileName() {
        String appDir = mContext.getFilesDir().getPath();
        return appDir + SETTINGS_FILE_NAME;
    }

    private boolean isMemoryLow() {
        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context. Cannot get memory info.");
            return false;
        }

        ActivityManager activityManager = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);

        if (activityManager == null) {
            Log.w(LOGTAG, "No activity manager. Cannot get memory info.");
            return false;
        }

        ActivityManager.MemoryInfo memInfo = new ActivityManager.MemoryInfo();
        activityManager.getMemoryInfo(memInfo);
        Log.d(LOGTAG, "Memory total: " + memInfo.totalMem + " avail: " + memInfo.availMem + " threshold: " + memInfo.threshold + " low: " + memInfo.lowMemory);
        return memInfo.lowMemory;
    }

    public static RemoteWorkManager getRemoteWorkManager(Context context) {
        return RemoteWorkManager.getInstance(context);
    }

    public static void startChecker(boolean wifiOnly) {
        startChecker(1, wifiOnly);
    }

    public static void stopChecker() {
        stopChecker(1);
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

        NetworkRequest.Builder networkRequestBuilder = new NetworkRequest.Builder()
            .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
            .addCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED);

        if (wifiOnly)
            networkRequestBuilder.addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_METERED);

        NetworkRequest networkRequest = networkRequestBuilder.build();

        Constraints constraints = new Constraints.Builder()
            .setRequiresBatteryNotLow(true)
            .setRequiredNetworkRequest(networkRequest, wifiOnly ? NetworkType.UNMETERED : NetworkType.CONNECTED)
            .build();

        PeriodicWorkRequest request = new PeriodicWorkRequest.Builder(
            NewMessageChecker.class, 15, TimeUnit.MINUTES)
                .setInitialDelay(1, TimeUnit.MINUTES)
                .setBackoffCriteria(BackoffPolicy.LINEAR, PeriodicWorkRequest.MIN_BACKOFF_MILLIS, TimeUnit.MILLISECONDS)
                .setConstraints(constraints)
                .build();

        Log.d(LOGTAG, "Backoff duration: " + PeriodicWorkRequest.MIN_BACKOFF_MILLIS + "ms");

        getRemoteWorkManager(context).enqueueUniquePeriodicWork(
            taskName,
            ExistingPeriodicWorkPolicy.REPLACE,
            request);
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
