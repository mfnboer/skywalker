// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "skywalker.h"

namespace Skywalker {

Skywalker::Skywalker(QObject* parent) :
    QObject(parent)
{}

void Skywalker::login(const QString user, QString password, const QString host)
{
    auto xrpc = std::make_unique<Xrpc::Client>(host);
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));
    mBsky->createSession(user, password,
        [this, user]{
            qDebug() << "Login" << user << "succeded";
            emit loginOk();
        },
        [this, user](const QString& error){
            qDebug() << "Login" << user << "failed:" << error;
            emit loginFailed(error);
        });
}

void Skywalker::getTimeline()
{
    Q_ASSERT(mBsky);
    mBsky->getTimeline(100, {},
       [this](auto feed){
            mTimelineModel.addFeed(std::move(feed->mFeed));
            emit timelineModelChanged();
       },
       [](const QString& error){ qDebug() << "getTimeline FAILED:" << error; }
    );
    // TODO: show error in GUI
}

}
