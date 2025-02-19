// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "virtual_keyboard_utils.h"
#include "android_utils.h"
#include "jni_callback.h"

namespace Skywalker {

bool VirtualKeyboardUtils::sInitialized = false;

VirtualKeyboardUtils::VirtualKeyboardUtils(QObject* parent) :
    QObject(parent)
{
    if (!sInitialized)
    {
        AndroidUtils::installVirtualKeyboardListener();
        sInitialized = true;
    }

    auto& jniCallbackListener = JNICallbackListener::getInstance();

    connect(&jniCallbackListener, &JNICallbackListener::keyboardHeightChanged,
        this, [this](int height){
            emit keyboardHeightChanged(height);
        });
}

}
