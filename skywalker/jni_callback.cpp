// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "jni_callback.h"
#include <QDebug>

#if defined(Q_OS_ANDROID)
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace Skywalker {

Q_GLOBAL_STATIC(std::unique_ptr<JNICallbackListener>, gTheInstance);

namespace {

#if defined(Q_OS_ANDROID)
void _handlePhotoPicked(JNIEnv* env, jobject, jint fd, jstring jsMimeType)
{
    QString mimeType = jsMimeType ? env->GetStringUTFChars(jsMimeType, nullptr) : QString();
    qDebug() << "Photo picked fd:" << fd << mimeType;
    auto& instance = *gTheInstance;
    if (instance)
        instance->handlePhotoPicked(fd, mimeType);
}

void _handlePhotoPickCanceled(JNIEnv*, jobject)
{
    qDebug() << "Photo pick canceled";
    auto& instance = *gTheInstance;
    if (instance)
        instance->handlePhotoPickCanceled();
}

void _handleEmojiPicked(JNIEnv* env, jobject, jstring jsEmoji)
{
    QString emoji = jsEmoji ? env->GetStringUTFChars(jsEmoji, nullptr) : QString();
    qDebug() << "Emoji picked:" << emoji;
    auto& instance = *gTheInstance;
    if (instance)
        instance->handleEmojiPicked(emoji);
}

void _handleVideoTranscodingOk(JNIEnv* env, jobject, jstring jsInputFileName, jstring jsOuputFileName, jint outputWidth, jint ouputHeight)
{
    QString inputFileName = jsInputFileName ? env->GetStringUTFChars(jsInputFileName, nullptr) : QString();
    QString outputFileName = jsOuputFileName ? env->GetStringUTFChars(jsOuputFileName, nullptr) : QString();
    qDebug() << "Video transcoding ok:" << outputFileName << "from:" << inputFileName;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleVideoTranscodingOk(inputFileName, outputFileName, (int)outputWidth, (int)ouputHeight);
}

void _handleVideoTranscodingFailed(JNIEnv* env, jobject, jstring jsInputFileName, jstring jsOuputFileName, jstring jsError)
{
    QString inputFileName = jsInputFileName ? env->GetStringUTFChars(jsInputFileName, nullptr) : QString();
    QString outputFileName = jsOuputFileName ? env->GetStringUTFChars(jsOuputFileName, nullptr) : QString();
    QString error = jsError ? env->GetStringUTFChars(jsError, nullptr) : QString();
    qDebug() << "Video transcoding failed:" << outputFileName << "from:" << inputFileName;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleVideoTranscodingFailed(inputFileName, outputFileName, error);
}

void _handleExtractTextAvailabilityOk(JNIEnv*, jobject, jint script, jboolean available)
{
    qDebug() << "Extract text availability ok, script:" << (int)script << "available:" << (bool)available;
    Q_ASSERT((int)script >= 0 && (int)script <= QEnums::Script::SCRIPT_LAST);
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleExtractTextAvailabilityOk(QEnums::Script((int)script), (bool)available);
}

void _handleExtractTextAvailabilityFailed(JNIEnv* env, jobject, jint script, jstring jsError)
{
    Q_ASSERT(script >= 0 && script <= QEnums::Script::SCRIPT_LAST);
    QString error = jsError ? env->GetStringUTFChars(jsError, nullptr) : QString();
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleExtractTextAvailabilityFailed(QEnums::Script((int)script), error);
}

void _handleExtractTextInstallProgress(JNIEnv*, jobject, jint script,  jint progressPercentage)
{
    Q_ASSERT(script >= 0 && script <= QEnums::Script::SCRIPT_LAST);
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleExtractTextInstallProgress(QEnums::Script((int)script), progressPercentage);
}

void _handleExtractTextInstallOk(JNIEnv*, jobject, jint script)
{
    Q_ASSERT(script >= 0 && script <= QEnums::Script::SCRIPT_LAST);
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleExtractTextInstallOk(QEnums::Script((int)script));
}

void _handleExtractTextInstallFailed(JNIEnv* env, jobject, jint script, jstring jsError)
{
    Q_ASSERT(script >= 0 && script <= QEnums::Script::SCRIPT_LAST);
    QString error = jsError ? env->GetStringUTFChars(jsError, nullptr) : QString();
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleExtractTextInstallFailed(QEnums::Script((int)script), error);
}

void _handleExtractTextOk(JNIEnv* env, jobject, jstring jsToken, jstring jsText)
{
    QString token = jsToken ? env->GetStringUTFChars(jsToken, nullptr) : QString();
    QString text = jsText ? env->GetStringUTFChars(jsText, nullptr) : QString();
    qDebug() << "Extract text ok:" << token << "text:" << text;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleExtractTextOk(token, text);
}

void _handleExtractTextFailed(JNIEnv* env, jobject, jstring jsToken, jstring jsError)
{
    QString token = jsToken ? env->GetStringUTFChars(jsToken, nullptr) : QString();
    QString error = jsError ? env->GetStringUTFChars(jsError, nullptr) : QString();
    qDebug() << "Extract text failed:" << token << "error:" << error;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleExtractTextFailed(token, error);
}

void _handleLanguageIdentified(JNIEnv* env, jobject, jstring jsLanguageCode, jint requestId)
{
    QString languageCode = jsLanguageCode ? env->GetStringUTFChars(jsLanguageCode, nullptr) : QString();
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleLanguageIdentified(languageCode, (int)requestId);
}

void _handleSharedTextReceived(JNIEnv* env, jobject, jstring jsSharedText)
{
    QString sharedText = jsSharedText ? env->GetStringUTFChars(jsSharedText, nullptr) : QString();
    qDebug() << "Shared text received:" << sharedText;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleSharedTextReceived(sharedText);
}

void _handleSharedImageReceived(JNIEnv* env, jobject, jstring jsContentUri, jstring jsText)
{
    QString fileName = jsContentUri ? env->GetStringUTFChars(jsContentUri, nullptr) : QString();
    qDebug() << "Shared image received:" << fileName;
    QString text = jsText ? env->GetStringUTFChars(jsText, nullptr) : QString();
    qDebug() << "Additional text received:" << text;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleSharedImageReceived(fileName, text);
}

void _handleSharedVideoReceived(JNIEnv* env, jobject, jstring jsContentUri, jstring jsText)
{
    QString fileName = jsContentUri ? env->GetStringUTFChars(jsContentUri, nullptr) : QString();
    qDebug() << "Shared video received:" << fileName;
    QString text = jsText ? env->GetStringUTFChars(jsText, nullptr) : QString();
    qDebug() << "Additional text received:" << text;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleSharedVideoReceived(fileName, text);
}

void _handleSharedDmTextReceived(JNIEnv* env, jobject, jstring jsSharedText)
{
    QString sharedText = jsSharedText ? env->GetStringUTFChars(jsSharedText, nullptr) : QString();
    qDebug() << "Shared DM text received:" << sharedText;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleSharedDmTextReceived(sharedText);
}

void _handleShowNotifications(JNIEnv*)
{
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleShowNotifications();
}

void _handleShowDirectMessages(JNIEnv*)
{
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleShowDirectMessages();
}

void _handleShowLink(JNIEnv* env, jobject, jstring jUri)
{
    QString uri = jUri ? env->GetStringUTFChars(jUri, nullptr) : QString();
    qDebug() << "Handling a show link request for: " << uri;
    auto& instance = *gTheInstance;

    if (instance) {
        instance->handleShowLink(uri);
    }
}

void _handleKeyboardHeightChanged(JNIEnv*, jobject, jint height)
{
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleKeyboardHeightChanged((int)height);
}

#endif

}

JNICallbackListener& JNICallbackListener::getInstance()
{
    auto& instance = *gTheInstance;
    if (!instance)
        instance.reset(new JNICallbackListener);

    return *instance;
}

void JNICallbackListener::handlePendingIntent()
{
#if defined(Q_OS_ANDROID)
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    activity.callMethod<void>("handlePendingIntent");
#endif
}

JNICallbackListener::JNICallbackListener() : QObject()
{
#if defined(Q_OS_ANDROID)
    QJniEnvironment jni;

    const JNINativeMethod photoPickerCallbacks[] = {
        { "emitPhotoPicked", "(ILjava/lang/String;)V", reinterpret_cast<void *>(_handlePhotoPicked) },
        { "emitPhotoPickCanceled", "()V", reinterpret_cast<void *>(_handlePhotoPickCanceled) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/QPhotoPicker", photoPickerCallbacks, 2);

    const JNINativeMethod emojiPickerCallbacks[] = {
        { "emitEmojiPicked", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handleEmojiPicked) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/EmojiPickerDialog", emojiPickerCallbacks, 1);

    const JNINativeMethod videoTranscoderCallbacks[] = {
        { "emitTranscodingOk", "(Ljava/lang/String;Ljava/lang/String;II)V", reinterpret_cast<void *>(_handleVideoTranscodingOk) },
        { "emitTranscodingFailed", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void *>(_handleVideoTranscodingFailed) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/VideoTranscoder", videoTranscoderCallbacks, 2);

    const JNINativeMethod textExtractorCallbacks[] = {
        {"emitCheckAvailabilityOk", "(IZ)V", reinterpret_cast<void *>(_handleExtractTextAvailabilityOk) },
        {"emitCheckAvailabilityFailed", "(ILjava/lang/String;)V", reinterpret_cast<void *>(_handleExtractTextAvailabilityFailed) },
        {"emitInstallModuleProgress", "(II)V", reinterpret_cast<void *>(_handleExtractTextInstallProgress) },
        {"emitInstallModuleOk", "(I)V", reinterpret_cast<void *>(_handleExtractTextInstallOk) },
        {"emitInstallModuleFailed", "(ILjava/lang/String;)V", reinterpret_cast<void *>(_handleExtractTextInstallFailed) },
        {"emitExtractOk", "(Ljava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void *>(_handleExtractTextOk) },
        {"emitExtractFailed", "(Ljava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void *>(_handleExtractTextFailed) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/TextExtractor", textExtractorCallbacks, 7);

    const JNINativeMethod languageDetectorCallbacks[] = {
        {"emitLanguageIdentified", "(Ljava/lang/String;I)V", reinterpret_cast<void *>(_handleLanguageIdentified) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/LanguageDetection", languageDetectorCallbacks, 1);

    const JNINativeMethod virtualKeyboardListenerCallbacks[] = {
        {"emitKeyboardHeightChanged", "(I)V", reinterpret_cast<void *>(_handleKeyboardHeightChanged) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/VirtualKeyboardListener", virtualKeyboardListenerCallbacks, 1);

    const JNINativeMethod skywalkerActivityCallbacks[] = {
        { "emitSharedTextReceived", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedTextReceived) },
        { "emitSharedImageReceived", "(Ljava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedImageReceived) },
        { "emitSharedVideoReceived", "(Ljava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedVideoReceived) },
        { "emitSharedDmTextReceived", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedDmTextReceived) },
        { "emitShowNotifications", "()V", reinterpret_cast<void *>(_handleShowNotifications) },
        { "emitShowDirectMessages", "()V", reinterpret_cast<void *>(_handleShowDirectMessages) },
        { "emitShowLink", "(Ljava/lang/String;)V",  reinterpret_cast<void *>(_handleShowLink) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/SkywalkerActivity", skywalkerActivityCallbacks, 7);
#endif
}

void JNICallbackListener::handlePhotoPicked(int fd, const QString& mimeType)
{
    emit photoPicked(fd, mimeType);
}

void JNICallbackListener::handlePhotoPickCanceled()
{
    emit photoPickCanceled();
}

void JNICallbackListener::handleEmojiPicked(const QString& emoji)
{
    emit emojiPicked(emoji);
}

void JNICallbackListener::handleVideoTranscodingOk(const QString& inputFileName, const QString& outputFileName, int outputWidth, int outputHeight)
{
    emit videoTranscodingOk(inputFileName, std::make_shared<FileSignal>(outputFileName), outputWidth, outputHeight);
}

void JNICallbackListener::handleVideoTranscodingFailed(const QString& inputFileName, const QString& outputFileName, const QString& error)
{
    emit videoTranscodingFailed(inputFileName, std::make_shared<FileSignal>(outputFileName), error);
}

void JNICallbackListener::handleExtractTextAvailabilityOk(QEnums::Script script, bool available)
{
    emit extractTextAvailabilityOk(script, available);
}

void JNICallbackListener::handleExtractTextAvailabilityFailed(QEnums::Script script, const QString& error)
{
    emit extractTextAvailabilityFailed(script, error);
}

void JNICallbackListener::handleExtractTextInstallProgress(QEnums::Script script, int progressPercentage)
{
    emit extractTextInstallProgress(script, progressPercentage);
}

void JNICallbackListener::handleExtractTextInstallOk(QEnums::Script script)
{
    emit extractTextInstallOk(script);
}

void JNICallbackListener::handleExtractTextInstallFailed(QEnums::Script script, const QString& error)
{
    emit extractTextInstallFailed(script, error);
}

void JNICallbackListener::handleExtractTextOk(const QString& imgSource, const QString& text)
{
    emit extractTextOk(imgSource, text);
}

void JNICallbackListener::handleExtractTextFailed(const QString& imgSource, const QString& error)
{
    emit extractTextFailed(imgSource, error);
}

void JNICallbackListener::handleLanguageIdentified(const QString& languageCode, int requestId)
{
    emit languageIdentified(languageCode, requestId);
}

void JNICallbackListener::handleSharedTextReceived(const QString& sharedText)
{
    emit sharedTextReceived(sharedText);
}

void JNICallbackListener::handleSharedImageReceived(const QString& contentUri, const QString& text)
{
    emit sharedImageReceived(contentUri, text);
}

void JNICallbackListener::handleSharedVideoReceived(const QString& contentUri, const QString& text)
{
    emit sharedVideoReceived(contentUri, text);
}

void JNICallbackListener::handleSharedDmTextReceived(const QString& sharedText)
{
    emit sharedDmTextReceived(sharedText);
}

void JNICallbackListener::handleShowNotifications()
{
    emit showNotifications();
}

void JNICallbackListener::handleShowDirectMessages()
{
    emit showDirectMessages();
}

void JNICallbackListener::handleShowLink(const QString& uri)
{
    emit showLink(uri);
}

void JNICallbackListener::handleKeyboardHeightChanged(int height)
{
    emit keyboardHeightChanged(height);
}

}
