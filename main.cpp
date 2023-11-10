// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "font_downloader.h"
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

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    const QUrl url(u"qrc:/Main.qml"_qs);
    engine.load(url);

    return app.exec();
}
