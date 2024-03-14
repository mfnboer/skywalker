// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "atproto_image_provider.h"
#include "font_downloader.h"
#include "shared_image_provider.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFont>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");

#ifdef Q_OS_ANDROID
    QJniEnvironment jniEnv;
    jclass javaClass = jniEnv.findClass("org/qtproject/qt/android/QtNative");
    qDebug() << "Java class:" << javaClass;
#endif

    qDebug() << "App instance:" << QCoreApplication::instance();

    QGuiApplication app(argc, argv);
    app.setOrganizationName("Skywalker");
    app.setApplicationName("Skywalker");

    Skywalker::FontDownloader::initAppFonts();

    QQmlApplicationEngine engine;
    auto* providerId = Skywalker::SharedImageProvider::SHARED_IMAGE;
    engine.addImageProvider(providerId, Skywalker::SharedImageProvider::getProvider(providerId));
    providerId = Skywalker::ATProtoImageProvider::DRAFT_IMAGE;
    engine.addImageProvider(providerId, Skywalker::ATProtoImageProvider::getProvider(providerId));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    const QUrl url(u"qrc:/skywalker/Main.qml"_qs);
    engine.load(url);

    return app.exec();
}
