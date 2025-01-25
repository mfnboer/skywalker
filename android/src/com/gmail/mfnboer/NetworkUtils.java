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
        NetworkCapabilities caps = getNetworkCapabilities();

        if (caps == null) {
            Log.w(LOGTAG, "No network capabilities.");
            return -1;
        }

        int kbps = caps.getLinkDownstreamBandwidthKbps();
        Log.d(LOGTAG, "Bandwidth: " + kbps + " kbps");
        return kbps;
    }

    public static boolean isUnmetered() {
        NetworkCapabilities caps = getNetworkCapabilities();

        if (caps == null) {
            Log.w(LOGTAG, "No network capabilities.");
            return false;
        }

        boolean unmetered = caps.hasCapability(NetworkCapabilities.NET_CAPABILITY_NOT_METERED);
        Log.d(LOGTAG, "Unmetered: " + unmetered);
        return unmetered;
    }

    private static NetworkCapabilities getNetworkCapabilities() {
        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context.");
            return null;
        }

        ConnectivityManager connectivityManager = context.getSystemService(ConnectivityManager.class);
        Network network = connectivityManager.getActiveNetwork();

        if (network == null) {
            Log.w(LOGTAG, "No active network.");
            return null;
        }

        NetworkCapabilities caps = connectivityManager.getNetworkCapabilities(network);
        return caps;
    }
}
