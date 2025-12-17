// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "camera_utils.h"
#include "android_utils.h"
#include "file_utils.h"
#include "temp_file_holder.h"

namespace Skywalker {

CameraUtils::CameraUtils(QObject* parent) : QObject(parent)
{
}

bool CameraUtils::checkCameraPermission()
{
#if defined(Q_OS_ANDROID)
    return AndroidUtils::checkPermission("android.permission.CAMERA");
#else
    return true;
#endif
}

QString CameraUtils::getCaptureFileName()
{
    auto tmpFile = FileUtils::makeTempFile("jpg", true);

    if (!tmpFile)
        return {};

    const QString fileName = tmpFile->fileName();
    TempFileHolder::instance().put(fileName);
    return fileName;
}

}
