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
void _handlePhotoPicked(JNIEnv* env, jobject, jstring jsUri)
{
    QString uri = jsUri ? env->GetStringUTFChars(jsUri, nullptr) : QString();
    qDebug() << "Photo picked:" << uri;
    auto& instance = *gTheInstance;
    if (instance)
        instance->handlePhotoPicked(uri);
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
        { "emitPhotoPicked", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handlePhotoPicked) },
        { "emitPhotoPickCanceled", "()V", reinterpret_cast<void *>(_handlePhotoPickCanceled) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/QPhotoPicker", photoPickerCallbacks, 2);

    const JNINativeMethod skywalkerActivityCallbacks[] = {
        { "emitSharedTextReceived", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handleSharedTextReceived) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/SkywalkerActivity", skywalkerActivityCallbacks, 1);
#endif
}

void JNICallbackListener::handlePhotoPicked(const QString contentUri)
{
    emit photoPicked(contentUri);
}

void JNICallbackListener::handlePhotoPickCanceled()
{
    emit photoPickCanceled();
}

void JNICallbackListener::handleSharedTextReceived(const QString sharedText)
{
    emit sharedTextReceived(sharedText);
}

}
