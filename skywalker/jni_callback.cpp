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

    const JNINativeMethod skywalkerActivityCallbacks[] = {
        { "emitSharedTextReceived", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedTextReceived) },
        { "emitSharedImageReceived", "(Ljava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedImageReceived) },
        { "emitSharedDmTextReceived", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedDmTextReceived) },
        { "emitShowNotifications", "()V", reinterpret_cast<void *>(_handleShowNotifications) },
        { "emitShowDirectMessages", "()V", reinterpret_cast<void *>(_handleShowDirectMessages) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/SkywalkerActivity", skywalkerActivityCallbacks, 5);
#endif
}

void JNICallbackListener::handlePhotoPicked(int fd, const QString mimeType)
{
    emit photoPicked(fd, mimeType);
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

void JNICallbackListener::handleSharedDmTextReceived(const QString sharedText)
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

}
