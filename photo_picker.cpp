// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "photo_picker.h"
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace Skywalker {

void pickPhoto()
{
#ifdef Q_OS_ANDROID
#if 0
    QJniObject ACTION_PICK = QJniObject::fromString("android.intent.action.GET_CONTENT");
    QJniObject intent("android/content/Intent");
    if (ACTION_PICK.isValid() && intent.isValid()) {
        intent.callObjectMethod("setAction", "(Ljava/lang/String;)Landroid/content/Intent;", ACTION_PICK.object());
        intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", QJniObject::fromString("image/*").object());

        QtAndroidPrivate::startActivity(intent, 101, nullptr);
    }
#endif
    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/QPhotoPicker",
                                       "start");
#endif
}

}
