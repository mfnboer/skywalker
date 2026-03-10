// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "atproto_image_provider.h"
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

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");
    //qputenv("QSG_INFO", "1");
    //qputenv("QSG_RENDER_TIMING", "1");

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
    providerId = Skywalker::ATProtoImageProvider::DRAFT_IMAGE;
    engine.addImageProvider(providerId, Skywalker::ATProtoImageProvider::getProvider(providerId));

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
