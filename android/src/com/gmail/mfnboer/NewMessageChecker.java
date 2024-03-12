// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import com.gmail.mfnboer.NewMessageNotifier;
import androidx.annotation.NonNull;
import androidx.work.Constraints;
import androidx.work.ExistingPeriodicWorkPolicy;
import androidx.work.Worker;
import androidx.work.WorkerParameters;
import androidx.work.WorkManager;
import androidx.work.PeriodicWorkRequest;
import android.content.Context;
import android.util.Log;
import java.lang.System;
import java.util.concurrent.TimeUnit;

public class NewMessageChecker extends Worker {
    private static final String LOGTAG = "NewMessageChecker";

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

    //public static native void emitCheckNewMessages();
    public static native void checkNewMessages();

    public NewMessageChecker(
        @NonNull Context context,
        @NonNull WorkerParameters params) {
        super(context, params);
        NewMessageNotifier.setContext(context);
    }

    @Override
    public Result doWork() {
        Log.d(LOGTAG, "Check for new messages");
        //emitCheckNewMessages();
        checkNewMessages();
        return Result.success();
    }

    public static void startChecker() {
        Log.d(LOGTAG, "Start checker, min interval: " + PeriodicWorkRequest.MIN_PERIODIC_INTERVAL_MILLIS);

        Constraints constraints = new Constraints.Builder().setRequiresBatteryNotLow(true).build();

        PeriodicWorkRequest request = new PeriodicWorkRequest.Builder(
            NewMessageChecker.class, 15, TimeUnit.MINUTES)
                .setInitialDelay(20, TimeUnit.SECONDS)
                .setConstraints(constraints).build();

        Context context = QtNative.getContext();
        WorkManager.getInstance(context).enqueueUniquePeriodicWork(
            "checkNewMessages",
            ExistingPeriodicWorkPolicy.KEEP,
            request);
    }

    public static void stopChecker() {
        Log.d(LOGTAG, "Stop checker");

        Context context = QtNative.getContext();
        WorkManager.getInstance(context).cancelUniqueWork("checkNewMessages");
    }
}
