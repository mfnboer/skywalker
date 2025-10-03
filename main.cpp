// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "atproto_image_provider.h"
#include "font_downloader.h"
#include "scaled_image_provider.h"
#include "shared_image_provider.h"
#include "skywalker.h"
#include "temp_file_holder.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFont>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");

    QGuiApplication app(argc, argv);
    app.setOrganizationName(Skywalker::Skywalker::APP_NAME);
    app.setApplicationName(Skywalker::Skywalker::APP_NAME);

    Skywalker::TempFileHolder::init();
    Skywalker::FontDownloader::initAppFonts();

    QQmlApplicationEngine engine;
    auto* providerId = Skywalker::SharedImageProvider::SHARED_IMAGE;
    engine.addImageProvider(providerId, Skywalker::SharedImageProvider::getProvider(providerId));
    providerId = Skywalker::ATProtoImageProvider::DRAFT_IMAGE;
    engine.addImageProvider(providerId, Skywalker::ATProtoImageProvider::getProvider(providerId));
    providerId = Skywalker::ScaledImageProvider::SCALED_IMAGE;
    engine.addImageProvider(providerId, Skywalker::ScaledImageProvider::getProvider(providerId));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    const QUrl url(u"qrc:/skywalker/qml/Main.qml"_s);
    engine.load(url);

    return app.exec();
}
