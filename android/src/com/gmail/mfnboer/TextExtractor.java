// Copyright (C) 2024 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;

import androidx.annotation.NonNull;

import android.graphics.Bitmap;
import android.graphics.Point;
import android.util.Log;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;
import com.google.mlkit.vision.common.InputImage;
import com.google.mlkit.vision.text.Text;
import com.google.mlkit.vision.text.TextRecognition;
import com.google.mlkit.vision.text.TextRecognizer;
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
    public static native void emitExtractOk(String token, String text);
    public static native void emitExtractFailed(String token, String error);

    public static void extractText(byte[] imgBytes, int width, int height, String token) {
        Log.d(LOGTAG, "Extract text: " + token);

        ByteBuffer imgBuffer = ByteBuffer.wrap(imgBytes);
        Bitmap imgBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        imgBitmap.copyPixelsFromBuffer(imgBuffer);

        // Latin
        TextRecognizer recognizer = TextRecognition.getClient(TextRecognizerOptions.DEFAULT_OPTIONS);
        InputImage image = InputImage.fromBitmap(imgBitmap, 0);

        Task<Text> result =
            recognizer.process(image)
                    .addOnSuccessListener(new OnSuccessListener<Text>() {
                        @Override
                        public void onSuccess(Text visionText) {
                            Log.d(LOGTAG, "Text extraction succeeded: " + visionText.getText());
                            String fullText = processText(visionText);
                            emitExtractOk(token, fullText);
                        }
                    })
                    .addOnFailureListener(
                        new OnFailureListener() {
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
            // the top-left corner of the block gives better results with indented lines.
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
