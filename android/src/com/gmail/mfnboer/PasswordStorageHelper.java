// Copyright (C) 2023 Michel de Boer
// License: GPLv3

package com.gmail.mfnboer;

import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;
import android.util.Log;

import java.security.KeyStore;
import java.security.Key;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.Certificate;
import javax.crypto.Cipher;

public class PasswordStorageHelper {
    private static final String LOGTAG = "PasswordStorageHelper";

    public static boolean initializeKeyStore(String keyAlias) {
        try {
            KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
            keyStore.load(null);
            if (!keyStore.containsAlias(keyAlias)) {
                KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(
                        KeyProperties.KEY_ALGORITHM_RSA, "AndroidKeyStore");

                KeyGenParameterSpec.Builder specBuilder = new KeyGenParameterSpec.Builder(
                        keyAlias,
                        KeyProperties.PURPOSE_ENCRYPT | KeyProperties.PURPOSE_DECRYPT)
                        .setBlockModes(KeyProperties.BLOCK_MODE_CBC)
                        .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_RSA_PKCS1)
                        .setRandomizedEncryptionRequired(false);

                keyPairGenerator.initialize(specBuilder.build());
                keyPairGenerator.generateKeyPair();
            }
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    // Store a password securely
    public static byte[] encryptPassword(String password, String keyAlias) {
        try {
            KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
            keyStore.load(null);

            PublicKey publicKey = keyStore.getCertificate(keyAlias).getPublicKey();

            Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
            cipher.init(Cipher.ENCRYPT_MODE, publicKey);

            byte[] encryptedPassword = cipher.doFinal(password.getBytes());
            return encryptedPassword;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    // Retrieve and decrypt the stored password
    public static String decryptPassword(byte[] encryptedPassword, String keyAlias) {
        try {
            KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
            keyStore.load(null);

            PrivateKey privateKey = (PrivateKey) keyStore.getKey(keyAlias, null);

            Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
            cipher.init(Cipher.DECRYPT_MODE, privateKey);
            byte[] decryptedBytes = cipher.doFinal(encryptedPassword);

            return new String(decryptedBytes);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
