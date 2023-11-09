// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "font_downloader.h"
#include "shared_image_provider.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFont>

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");

    QGuiApplication app(argc, argv);
    app.setOrganizationName("Skywalker");
    app.setApplicationName("Skywalker");

    Skywalker::FontDownloader::initAppFonts();

    QQmlApplicationEngine engine;

    engine.addImageProvider(Skywalker::SharedImageProvider::SHARED_IMAGE,
                            Skywalker::SharedImageProvider::getProvider(Skywalker::SharedImageProvider::SHARED_IMAGE));

    const QUrl url(u"qrc:/Main.qml"_qs);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
