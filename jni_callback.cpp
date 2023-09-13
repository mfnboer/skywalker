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
        { "emitPhotoPicked", "(Ljava/lang/String;)V", reinterpret_cast<void *>(_handlePhotoPicked) }
    };
    jni.registerNativeMethods("com/gmail/mfnboer/QPhotoPicker", photoPickerCallbacks, 1);
#endif
}

void JNICallbackListener::handlePhotoPicked(const QString contentUri)
{
    emit photoPicked(contentUri);
}

}
