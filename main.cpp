// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "font_downloader.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFont>

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");

    //qputenv("QT_SCALE_FACTOR", "0.9");
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Skywalker");
    app.setOrganizationDomain("skywalker.foo");
    app.setApplicationName("Skywalker");

    Skywalker::FontDownloader::initAppFonts();

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
