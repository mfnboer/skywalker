// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "font_downloader.h"
#include "shared_image_provider.h"
#include "sky_application.h"
#include "skywalker.h"
#include "temp_file_holder.h"
#include <QFont>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QQmlApplicationEngine>

using namespace Qt::Literals::StringLiterals;

#ifdef DEBUG
extern "C" const char* __lsan_default_options()
{
    return LEAK_SUPPRESSIONS;
}
#endif

#ifdef Q_OS_ANDROID
// HACK
// Google Play dashboard often shows ANR issues with accessibility in the stacktrace.
// Disable till we properly add accessibility.
// It disables all accessibility features. Copied from the following file
// and modified to always return false:
// c:\Qt6\6.10.1\Src\qtbase\src\plugins\platforms\android\qandroidplatformintegration.cpp
extern "C" {
    JNIEXPORT bool JNICALL
    Java_org_qtproject_qt_android_QtNativeAccessibility_accessibilitySupported(JNIEnv *, jobject)
    {
        qInfo() << "Disable accessibility";
        return false;
    }
}
#endif

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");

#ifdef DEBUG
    qputenv("QSG_INFO", "1");
    //qputenv("QSG_RENDER_TIMING", "1");
    //qputenv("QSG_VISUALIZE", "batches"); // batches, overdraw, clip, changes
#endif

#ifdef QT_NO_DEBUG_OUTPUT
    QLoggingCategory::setFilterRules("qml*.debug=false");
#endif

#ifdef DEBUG
    Skywalker::SkyApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif
    app.setOrganizationName(Skywalker::Skywalker::APP_NAME);
    app.setApplicationName(Skywalker::Skywalker::APP_NAME);

    Skywalker::TempFileHolder::init();
    Skywalker::FontDownloader::initAppFonts();
    qInfo() << "Image reader supported:" << QImageReader::supportedImageFormats();
    qInfo() << "Image writer supported:" << QImageWriter::supportedImageFormats();

    QQmlApplicationEngine engine;
    auto* providerId = Skywalker::SharedImageProvider::SHARED_IMAGE;
    engine.addImageProvider(providerId, Skywalker::SharedImageProvider::getProvider(providerId));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    const QUrl url(u"qrc:/skywalker/qml/Main.qml"_s);
    engine.load(url);

#ifdef Q_OS_ANDROID
    QNativeInterface::QAndroidApplication::hideSplashScreen(200);
#endif

    return app.exec();
}
