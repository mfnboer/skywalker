// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QFont>

int main(int argc, char *argv[])
{
    //qputenv("QT_SCALE_FACTOR", "0.9");
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Skywalker");
    app.setOrganizationDomain("skywalker.foo");
    app.setApplicationName("Skywalker");

    QLoggingCategory::setFilterRules("*.debug=false");
    qDebug() << "Font size:" << app.font();
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
