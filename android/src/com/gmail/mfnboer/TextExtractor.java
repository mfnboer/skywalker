// Copyright (C) 2024 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import com.gmail.mfnboer.SkywalkerApplication;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;

import androidx.annotation.NonNull;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.util.Log;

import com.google.android.gms.common.moduleinstall.InstallStatusListener;
import com.google.android.gms.common.moduleinstall.ModuleAvailabilityResponse;
import com.google.android.gms.common.moduleinstall.ModuleInstall;
import com.google.android.gms.common.moduleinstall.ModuleInstallClient;
import com.google.android.gms.common.moduleinstall.ModuleInstallRequest;
import com.google.android.gms.common.moduleinstall.ModuleInstallResponse;
import com.google.android.gms.common.moduleinstall.ModuleInstallStatusCodes;
import com.google.android.gms.common.moduleinstall.ModuleInstallStatusUpdate;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;
import com.google.mlkit.vision.common.InputImage;
import com.google.mlkit.vision.text.Text;
import com.google.mlkit.vision.text.TextRecognition;
import com.google.mlkit.vision.text.TextRecognizer;
import com.google.mlkit.vision.text.chinese.ChineseTextRecognizerOptions;
import com.google.mlkit.vision.text.devanagari.DevanagariTextRecognizerOptions;
import com.google.mlkit.vision.text.japanese.JapaneseTextRecognizerOptions;
import com.google.mlkit.vision.text.korean.KoreanTextRecognizerOptions;
import com.google.mlkit.vision.text.latin.TextRecognizerOptions;

class TextGroup {
    public boolean mNewGroup;
    public Text.TextBlock mTextBlock;

    TextGroup(boolean newGroup, Text.TextBlock textBlock) {
        mNewGroup = newGroup;
        mTextBlock = textBlock;
    }
}

public class TextExtractor {
    private static final String LOGTAG = "TextExtractor";

    // Values must be the same as QEnums::Script
    private static final int SCRIPT_LATIN = 0;
    private static final int SCRIPT_CHINESE = 1;
    private static final int SCRIPT_DEVANAGARI = 2;
    private static final int SCRIPT_JAPANESE = 3;
    private static final int SCRIPT_KOREAN = 4;

    public static native void emitCheckAvailabilityOk(int script, boolean available);
    public static native void emitCheckAvailabilityFailed(int script, String error);
    public static native void emitInstallModuleProgress(int script, int progressPercentage);
    public static native void emitInstallModuleOk(int script);
    public static native void emitInstallModuleFailed(int script, String error);
    public static native void emitExtractOk(String token, String text);
    public static native void emitExtractFailed(String token, String error);

    private static TextRecognizer createTextRecognizer(int script) {
        switch (script) {
            case SCRIPT_LATIN:
                return TextRecognition.getClient(TextRecognizerOptions.DEFAULT_OPTIONS);
            case SCRIPT_CHINESE:
                return TextRecognition.getClient(new ChineseTextRecognizerOptions.Builder().build());
            case SCRIPT_DEVANAGARI:
                return TextRecognition.getClient(new DevanagariTextRecognizerOptions.Builder().build());
            case SCRIPT_JAPANESE:
                return TextRecognition.getClient(new JapaneseTextRecognizerOptions.Builder().build());
            case SCRIPT_KOREAN:
                return TextRecognition.getClient(new KoreanTextRecognizerOptions.Builder().build());
            default:
                Log.w(LOGTAG, "Unknown script: " + script);
                return TextRecognition.getClient(TextRecognizerOptions.DEFAULT_OPTIONS);
        }
    }

    // This check automatically triggers an installation of the module if it is
    // not yet installed.
    public static void checkAvailability(int script) {
        Log.d(LOGTAG, "Check availability: " + script);
        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context");
            return;
        }

        TextRecognizer recognizer = createTextRecognizer(script);
        ModuleInstallClient moduleInstallClient = ModuleInstall.getClient(context);
        moduleInstallClient.areModulesAvailable(recognizer)
            .addOnSuccessListener(new OnSuccessListener<ModuleAvailabilityResponse>() {
                @Override
                public void onSuccess(ModuleAvailabilityResponse response) {
                    Log.d(LOGTAG, "Module: " + script + " availability: " + response.areModulesAvailable());
                    emitCheckAvailabilityOk(script, response.areModulesAvailable());
                }
            })
            .addOnFailureListener(new OnFailureListener() {
                @Override
                public void onFailure(@NonNull Exception e) {
                    Log.w(LOGTAG, "Module: " + script + " check availability failed: " + e.getMessage());
                    emitCheckAvailabilityFailed(script, e.getMessage());
                }
            });
    }

    public static void installModule(int script) {
        Log.d(LOGTAG, "Install module: " + script);
        Context context = SkywalkerApplication.getContext();

        if (context == null) {
            Log.w(LOGTAG, "No context");
            return;
        }

        ModuleInstallClient moduleInstallClient = ModuleInstall.getClient(context);
        TextRecognizer recognizer = createTextRecognizer(script);
        ModuleInstallRequest moduleInstallRequest = ModuleInstallRequest.newBuilder()
            .addApi(recognizer)
            .setListener(new InstallStatusListener() {
                @Override
                public void onInstallStatusUpdated(ModuleInstallStatusUpdate update) {
                    switch (update.getInstallState()) {
                        case ModuleInstallStatusUpdate.InstallState.STATE_COMPLETED:
                            Log.d(LOGTAG, "Module: " + script + " installation completed");
                            emitInstallModuleOk(script);
                            moduleInstallClient.unregisterListener(this);
                            break;
                        case ModuleInstallStatusUpdate.InstallState.STATE_CANCELED:
                            Log.d(LOGTAG, "Module: " + script + " installation canceled");
                            emitInstallModuleFailed(script, "installation canceled");
                            moduleInstallClient.unregisterListener(this);
                            break;
                        case ModuleInstallStatusUpdate.InstallState.STATE_FAILED:
                            String error = ModuleInstallStatusCodes.getStatusCodeString(update.getErrorCode());
                            Log.d(LOGTAG, "Module: " + script + " installation failed: " + error);
                            emitInstallModuleFailed(script, error);
                            moduleInstallClient.unregisterListener(this);
                            break;
                        default:
                            Log.d(LOGTAG, "Module: " + script + " state: " + update.getInstallState());
                            break;
                    }

                    ModuleInstallStatusUpdate.ProgressInfo progressInfo = update.getProgressInfo();

                    if (progressInfo != null && progressInfo.getTotalBytesToDownload() > 0) {
                        double progress = (double)progressInfo.getBytesDownloaded() / (double)progressInfo.getTotalBytesToDownload();
                        emitInstallModuleProgress(script, (int)(progress * 100));
                    }
                }
            })
            .build();

        moduleInstallClient.installModules(moduleInstallRequest)
            .addOnSuccessListener(new OnSuccessListener<ModuleInstallResponse>() {
                @Override
                public void onSuccess(ModuleInstallResponse response) {
                    Log.d(LOGTAG, "Module: " + script + " already installed: " + response.areModulesAlreadyInstalled());

                    if (response.areModulesAlreadyInstalled())
                        emitInstallModuleOk(script);
                    else
                        emitInstallModuleProgress(script, 0);
                }
            })
            .addOnFailureListener(new OnFailureListener() {
                @Override
                public void onFailure(@NonNull Exception e) {
                    Log.w(LOGTAG, "Module: " + script + " installation failed: " + e.getMessage());
                    emitInstallModuleFailed(script, e.getMessage());
                }
            });
    }

    public static void extractText(int script, byte[] imgBytes, int width, int height, String token) {
        Log.d(LOGTAG, "Extract text: " + token + " script: " + script);

        ByteBuffer imgBuffer = ByteBuffer.wrap(imgBytes);
        Bitmap imgBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        imgBitmap.copyPixelsFromBuffer(imgBuffer);

        TextRecognizer recognizer = createTextRecognizer(script);
        InputImage image = InputImage.fromBitmap(imgBitmap, 0);

        Task<Text> result = recognizer.process(image)
                .addOnSuccessListener(new OnSuccessListener<Text>() {
                    @Override
                    public void onSuccess(Text visionText) {
                        Log.d(LOGTAG, "Text extraction succeeded: " + visionText.getText());
                        String fullText = processText(visionText);
                        emitExtractOk(token, fullText);
                    }
                })
                .addOnFailureListener(new OnFailureListener() {
                    @Override
                    public void onFailure(@NonNull Exception e) {
                        Log.w(LOGTAG, "Text extraction failed: " + e.getMessage());
                        emitExtractFailed(token, e.getMessage());
                    }
                });
    }

    private static String processText(Text visionText) {
        if (visionText.getTextBlocks().isEmpty())
            return visionText.getText().replace('\n', ' ');

        String fullText = "";
        ArrayList<Text.TextBlock> textBlocks = new ArrayList(visionText.getTextBlocks());
        removeEmptyBlocks(textBlocks);

        if (textBlocks.isEmpty())
            return "";

        List<TextGroup> sortedTextGroups = spatialSort(textBlocks);

        for (TextGroup group : sortedTextGroups) {
            Log.d(LOGTAG, "Block: " + group.mTextBlock.getText());

            String blockText = group.mTextBlock.getText().replace('\n', ' ');

            if (fullText.isEmpty())
                fullText = blockText;
            else
                fullText = fullText + (group.mNewGroup ? "\n\n" : "\n") + blockText;
        }

        return fullText;
    }

    private static void removeEmptyBlocks(ArrayList<Text.TextBlock> blocks) {
        Iterator<Text.TextBlock> it = blocks.iterator();

        while (it.hasNext()) {
            Text.TextBlock block = it.next();

            if (block.getCornerPoints() == null || block.getCornerPoints().length < 4)
                it.remove();
            else if (block.getLines().isEmpty())
                it.remove();
            else if (block.getLines().get(0).getCornerPoints() == null || block.getLines().get(0).getCornerPoints().length < 4)
                it.remove();
        }
    }

    private static ArrayList<TextGroup> spatialSort(ArrayList<Text.TextBlock> blocks) {
        ArrayList<TextGroup> sortedGroups = new ArrayList<TextGroup>();
        Point anchor = new Point(0, 0);

        while (!blocks.isEmpty()) {
            Text.TextBlock block = removeClosest(anchor, blocks);
            TextGroup group;

            if (block == null) {
                block = removeClosest(new Point(0, 0), blocks);
                group = new TextGroup(true, block);
            } else {
                group = new TextGroup(false, block);
            }

            anchor = block.getCornerPoints()[3]; // bottom-left
            sortedGroups.add(group);
        }

        return sortedGroups;
    }

    private static Text.TextBlock removeClosest(Point p, ArrayList<Text.TextBlock> blocks) {
        int minDistance = Integer.MAX_VALUE;
        int closestIndex = 0;

        for (int i = 0; i < blocks.size(); ++i) {
            Text.TextBlock block = blocks.get(i);

            // Taking the distance to the top-left corner of the first line instead of
            // the top-left corner of the block gives better results with indented lines
            // at the start of a paragraph.
            int d = distance(p, block.getLines().get(0).getCornerPoints()[0]);

            if (d < minDistance) {
                minDistance = d;
                closestIndex = i;
            }
        }

        Text.TextBlock closestBlock = blocks.get(closestIndex);

        if (p.equals(new Point(0, 0))) {
            Log.d(LOGTAG, "New group, distance: " + minDistance + " corner: " + closestBlock.getCornerPoints()[0] + " " + closestBlock.getLines().get(0).getCornerPoints()[0] + " " + closestBlock.getText());
            blocks.remove(closestIndex);
            return closestBlock;
        }

        Text.Line line = closestBlock.getLines().get(0);
        Point[] linePoints = line.getCornerPoints();
        int lineHeight = distance(linePoints[0], linePoints[3]);
        Log.d(LOGTAG, "Closest to: " + p + " distance: " + minDistance + " lineHeight: " + lineHeight + " " + line.getText());

        // Heuristic: assume the text belongs to another block if
        // the distance is more than this.
        if (minDistance > 2 * lineHeight) {
            Log.d(LOGTAG, "Search for new group of blocks");
            return null;
        }

        blocks.remove(closestIndex);
        return closestBlock;
    }

    private static int distance(Point p, Point q) {
        int dx = q.x - p.x;
        int dy = q.y - p.y;
        return (int)Math.sqrt(dx * dx + dy * dy);
    }
}
