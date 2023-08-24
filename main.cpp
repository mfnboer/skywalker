// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include <QGuiApplication>
#include <QQmlApplicationEngine>


int main(int argc, char *argv[])
{
    //qputenv("QT_SCALE_FACTOR", "0.9");
    QGuiApplication app(argc, argv);
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/skywalker/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
