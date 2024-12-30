// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerApplication;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.util.Log;


public class NetworkUtils {
    private static final String LOGTAG = "NetworkUtils";

    public static int getBandwidthKbps() {
        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context.");
            return -1;
        }

        ConnectivityManager connectivityManager = context.getSystemService(ConnectivityManager.class);
        Network network = connectivityManager.getActiveNetwork();

        if (network == null) {
            Log.w(LOGTAG, "No active network.");
            return -1;
        }

        NetworkCapabilities caps = connectivityManager.getNetworkCapabilities(network);

        if (caps == null) {
            Log.w(LOGTAG, "Not network capabilities.");
            return -1;
        }

        int kbps = caps.getLinkDownstreamBandwidthKbps();
        Log.d(LOGTAG, "Bandwidth: " + kbps + " kbps");
        return kbps;
    }
}
