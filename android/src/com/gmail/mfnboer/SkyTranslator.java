// Copyright (C) 2026 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import android.util.Log;

import androidx.annotation.NonNull;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.mlkit.nl.translate.TranslateLanguage;
import com.google.mlkit.nl.translate.Translation;
import com.google.mlkit.nl.translate.Translator;
import com.google.mlkit.nl.translate.TranslatorOptions;
import com.google.mlkit.common.model.DownloadConditions;

public class SkyTranslator {
    private static final String LOGTAG = "SkyTranslator";

    public static native void emitTranslation(String text, int requestId);
    public static native void emitTranslationError(String error, int requestId);

    public static void translate(String text, String fromLangCode, String toLangCode, int requestId) {
        Log.d(LOGTAG, "Translate from: " + fromLangCode + " to: " + toLangCode + " request: " + requestId);
        String fromLang = TranslateLanguage.fromLanguageTag(fromLangCode);

        if (fromLang == null) {
            Log.w(LOGTAG, "Unsupported from-language: " + fromLangCode);
            emitTranslationError("Unsupported language: " + fromLangCode, requestId);
            return;
        }

        String toLang = TranslateLanguage.fromLanguageTag(toLangCode);

        if (toLang == null) {
            Log.w(LOGTAG, "Unsupported to-language: " + toLangCode);
            emitTranslationError("Unsupported language: " + toLangCode, requestId);
            return;
        }

        TranslatorOptions options = new TranslatorOptions.Builder()
            .setSourceLanguage(fromLang)
            .setTargetLanguage(toLang)
            .build();

        Translator translator = Translation.getClient(options);
        downloadModels(translator, text, requestId);
    }

    private static void downloadModels(Translator translator, String text, int requestId) {
        DownloadConditions conditions = new DownloadConditions.Builder()
            .requireWifi()
            .build();

        translator.downloadModelIfNeeded(conditions)
            .addOnSuccessListener(
                new OnSuccessListener<Void>() {
                  @Override
                  public void onSuccess(Void v) {
                    // Model downloaded successfully. Okay to start translating.
                    translateText(translator, text, requestId);
                  }
                })
            .addOnFailureListener(
                new OnFailureListener() {
                  @Override
                  public void onFailure(@NonNull Exception e) {
                    // Model couldn’t be downloaded or other internal error.
                    Log.w(LOGTAG, "Failed to download translation model: " + e.getMessage());
                    emitTranslationError(e.getMessage(), requestId);
                  }
                });
    }

    private static void translateText(Translator translator, String text, int requestId) {
        translator.translate(text)
            .addOnSuccessListener(
                new OnSuccessListener<String>() {
                  @Override
                  public void onSuccess(@NonNull String translatedText) {
                    // Translation successful.
                    emitTranslation(translatedText, requestId);
                  }
                })
            .addOnFailureListener(
                new OnFailureListener() {
                  @Override
                  public void onFailure(@NonNull Exception e) {
                    // Error.
                    Log.w(LOGTAG, "Failed to translate: " + e.getMessage());
                    emitTranslationError(e.getMessage(), requestId);
                  }
                });
    }
}
