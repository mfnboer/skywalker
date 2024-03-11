// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import androidx.annotation.NonNull;
import androidx.work.Constraints;
import androidx.work.ExistingPeriodicWorkPolicy;
import androidx.work.Worker;
import androidx.work.WorkerParameters;
import androidx.work.WorkManager;
import androidx.work.PeriodicWorkRequest;
import java.util.concurrent.TimeUnit;
import android.content.Context;
import android.util.Log;

public class NewMessageChecker extends Worker {
    private static final String LOGTAG = "NewMessageChecker";

    public NewMessageChecker(
        @NonNull Context context,
        @NonNull WorkerParameters params) {
        super(context, params);
    }

    @Override
    public Result doWork() {
        Log.d(LOGTAG, "Check for new messages");
        return Result.success();
    }

    public static void startChecker() {
        Log.d(LOGTAG, "Start checker");

        Constraints constraints = new Constraints.Builder().setRequiresBatteryNotLow(true).build();

        PeriodicWorkRequest request = new PeriodicWorkRequest.Builder(
            NewMessageChecker.class, 15, TimeUnit.MINUTES).setConstraints(constraints).build();

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
