// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "password_encryption.h"
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include "jni_utils.h"
#include <QJniObject>
#endif

namespace Skywalker
{

bool PasswordEncryption::init(const QString& keyAlias)
{
#ifdef Q_OS_ANDROID
    QJniEnvironment env;
    QJniObject keyAliasJni = QJniObject::fromString(keyAlias);
    auto [javaClass, methodId] = JniUtils::getClassAndMethod(
        env,
        "com/gmail/mfnboer/PasswordStorageHelper",
        "initializeKeyStore",
        "(Ljava/lang/String;)Z");

    if (!javaClass || !methodId)
        return false;

    auto initialized = QJniObject::callStaticMethod<jboolean>(
        javaClass,
        methodId,
        keyAliasJni.object<jstring>());

    mKeyAliasIntialized[keyAlias] = bool(initialized);
    qDebug() << "Init:" << keyAlias << bool(initialized);
#else
    qWarning() << "Encryption is only implemented for Android";
    mKeyAliasIntialized[keyAlias] = false;
#endif
    return mKeyAliasIntialized[keyAlias];
}

QByteArray PasswordEncryption::encrypt(const QString& token, const QString& keyAlias)
{
    if (!mKeyAliasIntialized[keyAlias])
    {
        qWarning() << "Key store not initialized";
        return {};
    }

#ifdef Q_OS_ANDROID
    QJniObject tokenJni = QJniObject::fromString(token);
    QJniObject keyAliasJni = QJniObject::fromString(keyAlias);

    auto encryptedTokenJni = QJniObject::callStaticMethod<jbyteArray>(
        "com/gmail/mfnboer/PasswordStorageHelper",
        "encryptPassword",
        "(Ljava/lang/String;Ljava/lang/String;)[B",
        tokenJni.object<jstring>(),
        keyAliasJni.object<jstring>());

    if (!encryptedTokenJni.isValid())
    {
        qWarning() << "Failed to encrypt token";
        return {};
    }

    QJniEnvironment env;
    jbyteArray encryptedTokenArray = encryptedTokenJni.object<jbyteArray>();

    if (!encryptedTokenArray)
    {
        qWarning() << "Failed to encrypt token";
        return {};
    }

    jsize arrayLength = env->GetArrayLength(encryptedTokenArray);

    if (arrayLength == 0)
    {
        qWarning() << "Failed to encrypt token";
        return {};
    }

    jboolean isCopy = false;
    jbyte* encryptedTokenBytes = env->GetByteArrayElements(encryptedTokenArray, &isCopy);
    QByteArray encryptedToken((char*)encryptedTokenBytes, arrayLength);
    return encryptedToken;
#else
    qWarning() << "Encryption is only implemented for Android";
    Q_UNUSED(token);
    return {};
#endif
}

QString PasswordEncryption::decrypt(const QByteArray& token, const QString& keyAlias) const
{
    const auto it = mKeyAliasIntialized.find(keyAlias);

    if (it == mKeyAliasIntialized.end() || !it->second)
    {
        qWarning() << "Key store not initialized";
        return {};
    }

#ifdef Q_OS_ANDROID
    QJniEnvironment env;
    auto tokenArray = env->NewByteArray(token.size());
    env->SetByteArrayRegion(tokenArray, 0, token.size(), (jbyte*)token.data());
    QJniObject keyAliasJni = QJniObject::fromString(keyAlias);

    auto decryptedToken = QJniObject::callStaticMethod<jstring>(
        "com/gmail/mfnboer/PasswordStorageHelper",
        "decryptPassword",
        "([BLjava/lang/String;)Ljava/lang/String;",
        tokenArray,
        keyAliasJni.object<jstring>());

    if (!decryptedToken.isValid())
    {
        qWarning() << "Failed to decrypt token";
        return {};
    }

    return decryptedToken.toString();
#else
    qWarning() << "Encryption is only implemented for Android";
    Q_UNUSED(token);
    return {};
#endif
}

}
