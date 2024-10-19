// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import java.io.IOException;
import java.nio.ByteBuffer;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.util.Log;
import android.view.Surface;

public class QVideoEncoder {
    private static final String LOGTAG = "spiralfun.QVideoEncoder";
    private static final String MIME_TYPE = "video/avc"; // H.264 Advanced Video Coding
    private static final int I_FRAME_INTERVAL = 5; // key frame interval, in seconds

    private int mWidth;
    private int mHeight;
    private Surface mInputSurface;
    private MediaCodec mEncoder;
    private MediaMuxer mMuxer;
    private int mTrackIndex;
    private boolean mMuxerStarted;
    private MediaCodec.BufferInfo mBufferInfo;
    private int mFrameDurationUs;
    private int mPresentationTimeUs;
    private Bitmap mFrameBitmap;

    public boolean init(String outputPath, int width, int height, int fps, int bitRate) {
        mWidth = width;
        mHeight = height;
        mFrameDurationUs = 1000000 / fps;
        mPresentationTimeUs = 0;
        mBufferInfo = new MediaCodec.BufferInfo();
        mFrameBitmap = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.ARGB_8888);

        // Configure the output format
        MediaFormat outputFormat = MediaFormat.createVideoFormat(MIME_TYPE, mWidth, mHeight);
        outputFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        outputFormat.setInteger(MediaFormat.KEY_BITRATE_MODE, MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR);
        outputFormat.setInteger(MediaFormat.KEY_BIT_RATE, bitRate);
        outputFormat.setInteger(MediaFormat.KEY_FRAME_RATE, fps);
        outputFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, I_FRAME_INTERVAL);

        // Create the encoder and configure it
        try {
            mEncoder = MediaCodec.createEncoderByType(MIME_TYPE);
        } catch (IOException e) {
            Log.w(LOGTAG, "Failed to create encoder: " + e.getMessage());
            return false;
        }

        MediaCodecInfo mci = mEncoder.getCodecInfo();
        MediaCodecInfo.EncoderCapabilities caps = mci.getCapabilitiesForType(MIME_TYPE).getEncoderCapabilities();
        Log.d(LOGTAG, "VBR: " + caps.isBitrateModeSupported(MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR));
        Log.d(LOGTAG, "Complexity: " + caps.getComplexityRange());
        Log.d(LOGTAG, "Quality: " + caps.getQualityRange());

        try {
            mEncoder.configure(outputFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        } catch (IllegalArgumentException e) {
            Log.w(LOGTAG, "Failed to configure encoder: " + e.getMessage());
            return false;
        }

        mInputSurface = mEncoder.createInputSurface();
        mEncoder.start();

        // Create the muxer and add a video track to it
        try {
            mMuxer = new MediaMuxer(outputPath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
        } catch (IOException e) {
            Log.w(LOGTAG, "Failed to create MediaMuxer: " + e.getMessage());
            return false;
        }

        mTrackIndex = -1;
        mMuxerStarted = false;

        return true;
    }

    private void release() {
        Log.d(LOGTAG, "release");

        if (mEncoder != null) {
            drainEncoder(true);
            mEncoder.stop();
            mEncoder.release();
            mEncoder = null;
        }
        if (mInputSurface != null) {
            mInputSurface.release();
            mInputSurface = null;
        }
        if (mMuxer != null) {
            mMuxer.stop();
            mMuxer.release();
            mMuxer = null;
        }
    }

    public boolean addFrame(byte[] frameArray) {
        if (!drainEncoder(false))
            return false;

        ByteBuffer frameBuffer = ByteBuffer.wrap(frameArray);
        mFrameBitmap.copyPixelsFromBuffer(frameBuffer);

        Canvas canvas = mInputSurface.lockCanvas(null);
        canvas.drawBitmap(mFrameBitmap, 0, 0, null);
        mInputSurface.unlockCanvasAndPost(canvas);

        return true;
    }

    /**
     * Extracts all pending data from the encoder.
     * <p>
     * If endOfStream is not set, this returns when there is no more data to drain.  If it
     * is set, we send EOS to the encoder, and then iterate until we see EOS on the output.
     * Calling this with endOfStream set should be done once, right before stopping the muxer.
     * Taken from: https://bigflake.com/mediacodec/EncodeAndMuxTest.java.txt
     * Made some adaptations
     */
    private boolean drainEncoder(boolean endOfStream) {
        final int TIMEOUT_USEC = 10000;

        if (endOfStream) {
            Log.d(LOGTAG, "sending EOS to encoder");
            mEncoder.signalEndOfInputStream();
        }

        ByteBuffer[] encoderOutputBuffers = mEncoder.getOutputBuffers();
        while (true) {
            int encoderStatus = mEncoder.dequeueOutputBuffer(mBufferInfo, TIMEOUT_USEC);
            if (encoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                // no output available yet
                if (!endOfStream) {
                    break;      // out of while
                } else {
                    Log.d(LOGTAG, "no output available, spinning to await EOS");
                }
            } else if (encoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                // not expected for an encoder
                encoderOutputBuffers = mEncoder.getOutputBuffers();
            } else if (encoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                // should happen before receiving buffers, and should only happen once
                if (mMuxerStarted) {
                    Log.e(LOGTAG, "format changed twice");
                    return false;
                }
                MediaFormat newFormat = mEncoder.getOutputFormat();
                Log.d(LOGTAG, "encoder output format changed: " + newFormat);

                // now that we have the Magic Goodies, start the muxer
                mTrackIndex = mMuxer.addTrack(newFormat);
                mMuxer.start();
                mMuxerStarted = true;
            } else if (encoderStatus < 0) {
                Log.w(LOGTAG, "unexpected result from encoder.dequeueOutputBuffer: " +
                        encoderStatus);
                // let's ignore it
            } else {
                ByteBuffer encodedData = encoderOutputBuffers[encoderStatus];
                if (encodedData == null) {
                    Log.e(LOGTAG, "encoderOutputBuffer " + encoderStatus + " was null");
                    return false;
                }

                if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                    // The codec config data was pulled out and fed to the muxer when we got
                    // the INFO_OUTPUT_FORMAT_CHANGED status.  Ignore it.
                    Log.d(LOGTAG, "ignoring BUFFER_FLAG_CODEC_CONFIG");
                    mBufferInfo.size = 0;
                }

                if (mBufferInfo.size != 0) {
                    if (!mMuxerStarted) {
                        Log.e(LOGTAG, "muxer hasn't started");
                        return false;
                    }

                    mBufferInfo.presentationTimeUs = mPresentationTimeUs;
                    mPresentationTimeUs += mFrameDurationUs;

                    // adjust the ByteBuffer values to match BufferInfo (not needed?)
                    encodedData.position(mBufferInfo.offset);
                    encodedData.limit(mBufferInfo.offset + mBufferInfo.size);

                    mMuxer.writeSampleData(mTrackIndex, encodedData, mBufferInfo);
                }

                mEncoder.releaseOutputBuffer(encoderStatus, false);

                if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    if (!endOfStream) {
                        Log.w(LOGTAG, "reached end of stream unexpectedly");
                    } else {
                        Log.d(LOGTAG, "end of stream reached");
                    }
                    break;      // out of while
                }
            }
        }

        return true;
    }
}
