// Copyright (C) 2024 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import java.util.List;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import android.util.Log;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.mlkit.nl.languageid.IdentifiedLanguage;
import com.google.mlkit.nl.languageid.LanguageIdentification;
import com.google.mlkit.nl.languageid.LanguageIdentificationOptions;
import com.google.mlkit.nl.languageid.LanguageIdentifier;

public class LanguageDetection {
    private static final String LOGTAG = "LanguageDetection";

    public static native void emitLanguageIdentified(String languageCode, int requestId);

    public static void detectLanguage(String text, int requestId) {
        Log.d(LOGTAG, "Identify language: " + text + " request: " + requestId + " length:" + text.length());
        float confidenceThreshold = calcConfidenceThreshold(text);

        LanguageIdentifier languageIdentifier = LanguageIdentification.getClient(
            new LanguageIdentificationOptions.Builder()
                .setConfidenceThreshold(confidenceThreshold)
                .build());

        languageIdentifier.identifyLanguage(text)
            .addOnSuccessListener(
                new OnSuccessListener<String>() {
                    @Override
                    public void onSuccess(@Nullable String languageCode) {
                        Log.d(LOGTAG, "Identified language code: " + languageCode);
                        emitLanguageIdentified(languageCode, requestId);
                    }
                })
            .addOnFailureListener(
                new OnFailureListener() {
                    @Override
                    public void onFailure(@NonNull Exception e) {
                        Log.w(LOGTAG, "Failed to identify language: " + e.getMessage());
                        emitLanguageIdentified(null, requestId);
                    }
                });
    }

    private static float calcConfidenceThreshold(String text) {
        int l = text.length();

        if (l < 30)
            return 0.99f;
        if (l < 35)
            return 0.98f;
        if (l < 40)
            return 0.97f;
        if (l < 50)
            return 0.95f;
        if (l < 60)
            return 0.90f;
        if (l < 70)
            return 0.85f;

        return 0.80f;
    }
}
