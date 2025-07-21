// Copyright (C) 2025 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerApplication;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutionException;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;

import com.google.common.util.concurrent.ListenableFuture;
import com.google.mlkit.genai.common.DownloadCallback;
import com.google.mlkit.genai.common.FeatureStatus;
import com.google.mlkit.genai.common.GenAiException;
import com.google.mlkit.genai.imagedescription.ImageDescriber;
import com.google.mlkit.genai.imagedescription.ImageDescriberOptions;
import com.google.mlkit.genai.imagedescription.ImageDescription;
import com.google.mlkit.genai.imagedescription.ImageDescriptionRequest;
import com.google.mlkit.genai.imagedescription.ImageDescriptionResult;

public class AltTextGenerator {
    private static final String LOGTAG = "AltTextGenerator";
    private static ImageDescriber sImageDescriber = null;

    public static boolean prepareAndStartImageDescription(byte[] imgBytes, int width, int height) {
        Log.d(LOGTAG, "Generate alt text, width: " + width + " height: " + height);
        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context");
            return false;
        }

        ImageDescriberOptions options = ImageDescriberOptions.builder(context).build();
        sImageDescriber = ImageDescription.getClient(options);

        // Check feature availability, status will be one of the following:
        // UNAVAILABLE, DOWNLOADABLE, DOWNLOADING, AVAILABLE
        try {
            int featureStatus = sImageDescriber.checkFeatureStatus().get();

            ByteBuffer imgBuffer = ByteBuffer.wrap(imgBytes);
            Bitmap imgBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            imgBitmap.copyPixelsFromBuffer(imgBuffer);

            if (featureStatus == FeatureStatus.DOWNLOADABLE) {
                Log.d(LOGTAG, "ImagaDescriber downloadable");

                // Download feature if necessary.
                // If downloadFeature is not called, the first inference request
                // will also trigger the feature to be downloaded if it's not
                // already downloaded.
                sImageDescriber.downloadFeature(new DownloadCallback() {
                    @Override
                    public void onDownloadCompleted() {
                        startImageDescriptionRequest(imgBitmap);
                    }

                    @Override
                    public void onDownloadFailed(GenAiException e) {}

                    @Override
                    public void onDownloadProgress(long totalBytesDownloaded) {}

                    @Override
                    public void onDownloadStarted(long bytesDownloaded) {}
                });
            } else if (featureStatus == FeatureStatus.DOWNLOADING) {
                Log.d(LOGTAG, "ImagaDescriber downloading");

                // Inference request will automatically run once feature is
                // downloaded.
                // If Gemini Nano is already downloaded on the device, the
                // feature-specific LoRA adapter model will be downloaded
                // very quickly. However, if Gemini Nano is not already
                // downloaded, the download process may take longer.
                startImageDescriptionRequest(imgBitmap);
            } else if (featureStatus == FeatureStatus.AVAILABLE) {
                Log.d(LOGTAG, "ImagaDescriber available");
                startImageDescriptionRequest(imgBitmap);
            }
            else if (featureStatus == FeatureStatus.UNAVAILABLE) {
                Log.w(LOGTAG, "ImageDescriber not available");
                return false;
            }
        } catch (ExecutionException | InterruptedException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    private static void startImageDescriptionRequest(Bitmap bitmap) {
        // Create task request
        ImageDescriptionRequest imageDescriptionRequest =
                ImageDescriptionRequest.builder(bitmap).build();

        // Start image description request with streaming response
        sImageDescriber.runInference(imageDescriptionRequest, newText -> {
            // Append new output text to show in UI
            // This callback is called incrementally as the description
            // is generated
            Log.d(LOGTAG, "ALT: " + newText);
        });
    }
}
