// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import java.lang.String;
import java.io.FileNotFoundException;
import android.util.Log;
import android.content.Context;
import android.content.ContentResolver;
import android.net.Uri;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.ParcelFileDescriptor;
import androidx.core.provider.FontRequest;
import androidx.core.provider.FontsContractCompat;

public class GMSEmojiFontDownloader {
    private static final String LOGTAG = "GMSEmojiFontDownloader";

    // GMS fonts provider data
    private static final String PROVIDER_AUTHORITY = "com.google.android.gms.fonts";
    private static final String PROVIDER_PACKAGE = "com.google.android.gms";

    // Emoji font search query (copied from EmojiCompat source)
    private static final String EMOJI_QUERY = "emojicompat-emoji-font";

    // Font Certificates resources strings (from fonts_certs.xml)
    private static final String FONT_CERTIFICATE_ID = "com_google_android_gms_fonts_certs";
    private static final String FONT_CERTIFICATE_TYPE = "array";

    public static int getFontFileDescriptor() {
        Context context = QtNative.getContext();

        // obtain id for the font_certs.xml
        int certificateId = context.getResources().getIdentifier(
                                      FONT_CERTIFICATE_ID,
                                      FONT_CERTIFICATE_TYPE,
                                      context.getPackageName());

        // creating the request
        FontRequest request = new FontRequest(
                                      PROVIDER_AUTHORITY,
                                      PROVIDER_PACKAGE,
                                      EMOJI_QUERY,
                                      certificateId);

        // fetch the font
        FontsContractCompat.FontFamilyResult result;
        try {
            result = FontsContractCompat.fetchFonts(context, null, request);
        } catch (NameNotFoundException e) {
            Log.w(LOGTAG, e.getMessage());
            return -1;
        }

        final FontsContractCompat.FontInfo[] fontInfos = result.getFonts();
        final Uri emojiFontUri = fontInfos[0].getUri();

        final ContentResolver resolver = context.getContentResolver();
        // in this case the Font URI is always a content scheme file, made
        // so the app requesting it has permissions to open
        final ParcelFileDescriptor fileDescriptor;
        try {
            fileDescriptor = resolver.openFileDescriptor(fontInfos[0].getUri(), "r");
        } catch (FileNotFoundException e) {
            Log.w(LOGTAG, e.getMessage());
            return -1;
        }

        // the detachFd will return a native file descriptor that we must close
        // later in C++ code
        int fd = fileDescriptor.detachFd();
        return fd;
    }
}
