// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "jni_callback.h"
#include <QDebug>

#if defined(Q_OS_ANDROID)
#include <QJniObject>
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
#endif

}

JNICallbackListener& JNICallbackListener::getInstance()
{
    auto& instance = *gTheInstance;
    if (!instance)
        instance.reset(new JNICallbackListener);

    return *instance;
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

}
