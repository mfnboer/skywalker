// Copyright (C) 2024 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerApplication;

import java.io.File;

import androidx.media3.common.MediaItem;
import androidx.media3.common.MimeTypes;
import androidx.media3.effect.Presentation;
import androidx.media3.transformer.Composition;
import androidx.media3.transformer.EditedMediaItem;
import androidx.media3.transformer.Effects;
import androidx.media3.transformer.ExportException;
import androidx.media3.transformer.ExportResult;
import androidx.media3.transformer.InAppMp4Muxer;
import androidx.media3.transformer.Transformer;
import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.google.common.collect.ImmutableList;

public class VideoTranscoder {
    private static final String LOGTAG = "VideoTranscoder";
    public static native void emitTranscodingOk(String inputFilePath, String outputFilePath, int outputWidth, int outputHeight);
    public static native void emitTranscodingFailed(String inputFilePath, String outputFilePath, String error);

    public static void transcodeVideo(String inputFilePath, String outputFilePath, int height, int startMs, int endMs, boolean removeAudio) {
        Log.d(LOGTAG, "Transcode video, in: " + inputFilePath + " out: " + outputFilePath + " height: " + height + " start: " + startMs + " end: " + endMs + " remove audio: " + removeAudio);

        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context");
            return;
        }

        Uri inputUri = Uri.fromFile(new File(inputFilePath));

        MediaItem.Builder mediaItemBuilder = new MediaItem.Builder()
                .setUri(inputUri);

        if (startMs >= 0 && endMs > startMs) {
            mediaItemBuilder.setClippingConfiguration(
                new MediaItem.ClippingConfiguration.Builder()
                    .setStartPositionMs(startMs)
                    .setEndPositionMs(endMs)
                    .build());
        }

        MediaItem mediaItem = mediaItemBuilder.build();
        EditedMediaItem.Builder emiBuilder = new EditedMediaItem.Builder(mediaItem);

        if (height > 0)
            emiBuilder.setEffects(new Effects(ImmutableList.of(), ImmutableList.of(Presentation.createForHeight(height))));

        if (removeAudio)
            emiBuilder.setRemoveAudio(true);

        EditedMediaItem editedMediaItem = emiBuilder.build();

        // NOTE: The InAppMuxer is needed to transfrom mpeg-ts to mp4. Without it the code crashes.
        Transformer transformer = new Transformer.Builder(context)
                .setVideoMimeType(MimeTypes.VIDEO_H264)
                .setAudioMimeType(MimeTypes.AUDIO_AAC)
                .setMuxerFactory(new InAppMp4Muxer.Factory())
                .addListener(new Transformer.Listener() {
                    @Override
                    public void onCompleted(Composition composition, ExportResult result) {
                        final boolean rotated = (result.width == height);
                        final int w = rotated ? result.height : result.width;
                        final int h = rotated ? result.width : result.height;
                        Log.d(LOGTAG, "Transcoding completed: " + outputFilePath + " size: " + result.fileSizeBytes + " width: " + w + " height: " + h + " rotated: " + rotated);
                        emitTranscodingOk(inputFilePath, outputFilePath, w, h);
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
