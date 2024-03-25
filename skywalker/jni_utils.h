// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

namespace Skywalker::JniUtils {

#ifdef Q_OS_ANDROID
// Instead of passing class name, method name and signature directly to QJniObject::callStaticMethod
// use this function to get calls and method from a specific JNI env when you call from
// a background task.
std::tuple<jclass, jmethodID> getClassAndMethod(QJniEnvironment& env, const char *className, const char *methodName, const char *signature);
#endif

}
