// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "jni_utils.h"

namespace Skywalker::JniUtils {

#ifdef Q_OS_ANDROID
std::tuple<jclass, jmethodID> getClassAndMethod(QJniEnvironment& env, const char *className, const char *methodName, const char *signature)
{
    jclass javaClass = env.findClass(className);

    if (!javaClass)
    {
        qWarning() << "Could not find class:" << className;
        return { nullptr, nullptr };
    }

    jmethodID methodId = env.findStaticMethod(javaClass, methodName, signature);

    if (!methodId)
    {
        qWarning() << "Could not find method:" << methodName;
        return { nullptr, nullptr };
    }

    return { javaClass, methodId };
}
#endif

}
