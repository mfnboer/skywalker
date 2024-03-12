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
void _handlePhotoPicked(JNIEnv*, jobject, jint fd)
{
    qDebug() << "Photo picked fd:" << fd;
    auto& instance = *gTheInstance;
    if (instance)
        instance->handlePhotoPicked(fd);
}

void _handlePhotoPickCanceled(JNIEnv*, jobject)
{
    qDebug() << "Photo pick canceled";
    auto& instance = *gTheInstance;
    if (instance)
        instance->handlePhotoPickCanceled();
}

void _handleSharedTextReceived(JNIEnv* env, jobject, jstring jsSharedText)
{
    QString sharedText = jsSharedText ? env->GetStringUTFChars(jsSharedText, nullptr) : QString();
    qDebug() << "Shared text received:" << sharedText;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleSharedTextReceived(sharedText);
}

void _handleSharedImageReceived(JNIEnv* env, jobject, jstring jsFileName, jstring jsText)
{
    QString fileName = jsFileName ? env->GetStringUTFChars(jsFileName, nullptr) : QString();
    qDebug() << "Shared image received:" << fileName;
    QString text = jsText ? env->GetStringUTFChars(jsText, nullptr) : QString();
    qDebug() << "Additional text received:" << text;
    auto& instance = *gTheInstance;

    if (instance)
        instance->handleSharedImageReceived(fileName, text);
}

void _handlePause(JNIEnv*)
{
    auto& instance = *gTheInstance;

    if (instance)
        instance->handlePause();
}

// void _handleCheckNewMessages(JNIEnv*)
// {
//     auto& instance = *gTheInstance;

//     if (instance)
//         instance->handleCheckNewMessages();
// }

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
        { "emitPhotoPicked", "(I)V", reinterpret_cast<void *>(_handlePhotoPicked) },
        { "emitPhotoPickCanceled", "()V", reinterpret_cast<void *>(_handlePhotoPickCanceled) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/QPhotoPicker", photoPickerCallbacks, 2);

    const JNINativeMethod skywalkerActivityCallbacks[] = {
        { "emitSharedTextReceived", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedTextReceived) },
        { "emitSharedImageReceived", "(Ljava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedImageReceived) },
        { "emitPause", "()V", reinterpret_cast<void *>(_handlePause) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/SkywalkerActivity", skywalkerActivityCallbacks, 3);

    // const JNINativeMethod newMessageCheckerCallbacks[] = {
    //     { "emitCheckNewMessages", "()V", reinterpret_cast<void *>(_handleCheckNewMessages) }
    // };
    // jni.registerNativeMethods("com/gmail/mfnboer/NewMessageChecker", newMessageCheckerCallbacks, 1);
#endif
}

void JNICallbackListener::handlePhotoPicked(int fd)
{
    emit photoPicked(fd);
}

void JNICallbackListener::handlePhotoPickCanceled()
{
    emit photoPickCanceled();
}

void JNICallbackListener::handleSharedTextReceived(const QString sharedText)
{
    emit sharedTextReceived(sharedText);
}

void JNICallbackListener::handleSharedImageReceived(const QString fileName, const QString text)
{
    emit sharedImageReceived(fileName, text);
}

void JNICallbackListener::handlePause() {
    emit appPause();
}

void JNICallbackListener::handleCheckNewMessages() {
    emit checkNewMessages();
}

}
