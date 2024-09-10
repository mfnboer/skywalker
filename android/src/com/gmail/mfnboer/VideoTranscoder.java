// Copyright (C) 2024 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import java.io.File;

import androidx.media3.common.MediaItem;
import androidx.media3.effect.Presentation;
import androidx.media3.transformer.Composition;
import androidx.media3.transformer.EditedMediaItem;
import androidx.media3.transformer.Effects;
import androidx.media3.transformer.ExportException;
import androidx.media3.transformer.ExportResult;
import androidx.media3.transformer.Transformer;
import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.google.common.collect.ImmutableList;

public class VideoTranscoder {
    private static final String LOGTAG = "VideoTranscoder";
    public static native void emitTranscodingOk(String inputFilePath, String outputFilePath);
    public static native void emitTranscodingFailed(String inputFilePath, String outputFilePath, String error);

    public static void transcodeVideo(String inputFilePath, String outputFilePath, int height) {
        Log.d(LOGTAG, "Transcode video, in: " + inputFilePath + " out: " + outputFilePath + " height: " + height);

        Context context = QtNative.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context");
            return;
        }

        Uri inputUri = Uri.fromFile(new File(inputFilePath));

        EditedMediaItem editedMediaItem = new EditedMediaItem.Builder(MediaItem.fromUri(inputUri))
                .setEffects(new Effects(ImmutableList.of(), ImmutableList.of(Presentation.createForHeight(height))))
                .build();


        Transformer transformer = new Transformer.Builder(context)
                .addListener(new Transformer.Listener() {
                    @Override
                    public void onCompleted(Composition composition, ExportResult result) {
                        Log.d(LOGTAG, "Transcoding completed: " + outputFilePath + " size:" + result.fileSizeBytes);
                        emitTranscodingOk(inputFilePath, outputFilePath);
                    }

                    @Override
                    public void onError(Composition composition, ExportResult result, ExportException exception) {
                        String error = exception.getErrorCodeName();
                        Log.w(LOGTAG, "Transcoding failed: " + error);
                        emitTranscodingFailed(inputFilePath, outputFilePath, error);
                    }
                })
                .build();

        transformer.start(editedMediaItem, outputFilePath);
    }
}
