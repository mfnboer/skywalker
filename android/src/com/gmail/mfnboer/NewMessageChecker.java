// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import com.gmail.mfnboer.NewMessageNotifier;

import androidx.annotation.NonNull;
import androidx.work.Configuration;
import androidx.work.Constraints;
import androidx.work.ExistingPeriodicWorkPolicy;
import androidx.work.NetworkType;
import androidx.work.Worker;
import androidx.work.WorkerParameters;
import androidx.work.WorkManager;
import androidx.work.PeriodicWorkRequest;
import androidx.work.multiprocess.RemoteWorkManager;

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
            System.loadLibrary("appskywalker_arm64-v8a");
        } catch (UnsatisfiedLinkError ule) {
            try {
                System.loadLibrary("appskywalker_armeabi-v7a");
            }
            catch (Exception e) {
                Log.w(LOGTAG, "Cannot load appskywalker_armeabi-v7a lib: " + e.getMessage());
            }
        } catch (Exception e) {
            Log.w(LOGTAG, "Cannot load appskywalker_arm64-v8a lib: " + e.getMessage());
        }
    }

    public static native void checkNewMessages(String settingsFileName, String libDir);

    private Context mContext;

    public NewMessageChecker(
        @NonNull Context context,
        @NonNull WorkerParameters params) {
        super(context, params);
        mContext = context;
    }

    @Override
    public Result doWork() {
        Log.d(LOGTAG, "Check for new messages");
        Log.d(LOGTAG, "data dir: " + mContext.getDataDir().getPath());

        ApplicationInfo appInfo = mContext.getApplicationInfo();
        Log.d(LOGTAG, "native lib dir: " + appInfo.nativeLibraryDir);
        Log.d(LOGTAG, "source dir:" + appInfo.sourceDir);

        NewMessageNotifier.setContext(mContext);
        checkNewMessages(getSettingsFileName(), appInfo.nativeLibraryDir);
        return Result.success();
    }

    private String getSettingsFileName() {
        String appDir = mContext.getFilesDir().getPath();
        return appDir + SETTINGS_FILE_NAME;
    }

    public static RemoteWorkManager getRemoteWorkManager(Context context) {
        return RemoteWorkManager.getInstance(context);
    }

    public static void startChecker() {
        Log.d(LOGTAG, "Start checker, min interval: " + PeriodicWorkRequest.MIN_PERIODIC_INTERVAL_MILLIS);

        Constraints constraints = new Constraints.Builder()
            .setRequiresBatteryNotLow(true)
            .setRequiredNetworkType(NetworkType.CONNECTED)
            .build();

        PeriodicWorkRequest request = new PeriodicWorkRequest.Builder(
            NewMessageChecker.class, 15, TimeUnit.MINUTES)
                .setInitialDelay(20, TimeUnit.SECONDS)
                .setConstraints(constraints).build();

        Context context = QtNative.getContext();
        getRemoteWorkManager(context).enqueueUniquePeriodicWork(
            "checkNewMessages",
            ExistingPeriodicWorkPolicy.REPLACE,
            request);
    }

    public static void stopChecker() {
        Log.d(LOGTAG, "Stop checker");

        Context context = QtNative.getContext();
        getRemoteWorkManager(context).cancelUniqueWork("checkNewMessages");
    }
}
