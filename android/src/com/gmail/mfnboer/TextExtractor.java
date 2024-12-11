// Copyright (C) 2024 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import org.qtproject.qt.android.QtNative;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Collections;
import java.util.Comparator;

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

class TextBlockComparator implements Comparator<Text.TextBlock> {
    public int compare(Text.TextBlock lhs, Text.TextBlock rhs) {
        Point lhsPoint = lhs.getCornerPoints()[0];
        Point rhsPoint = rhs.getCornerPoints()[0];

        if (lhsPoint.y != rhsPoint.y)
            return lhsPoint.y - rhsPoint.y;

        return lhsPoint.x - rhsPoint.x;
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
        List<Text.TextBlock> sortedTextBlocks = new ArrayList(visionText.getTextBlocks());
        Collections.sort(sortedTextBlocks, new TextBlockComparator());

        for (Text.TextBlock block : sortedTextBlocks) {
            Log.d(LOGTAG, "Block: " + block.getText());

            String blockText = block.getText().replace('\n', ' ');

            if (fullText.isEmpty())
                fullText = blockText;
            else
                fullText = fullText + "\n\n" + blockText;
        }

        return fullText;
    }
}
