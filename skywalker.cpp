// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "skywalker.h"

namespace Skywalker {

Skywalker::Skywalker(QObject* parent) :
    QObject(parent)
{}

void Skywalker::login(const QString user, QString password, const QString host)
{
    qDebug() << "Login:" << user << "host:" << host;
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

void Skywalker::getTimeline(const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline:" << cursor;

    if (mGetTimelineInProgress)
    {
        qDebug() << "Get timeline still in progress";
        return;
    }

    std::optional<QString> cur;
    if (!cursor.isEmpty())
        cur = cursor;

    setGetTimelineInProgress(true);
    mBsky->getTimeline({}, cur,
       [this ,cursor](auto feed){
            if (cursor.isEmpty())
                mTimelineModel.setFeed(std::move(feed));
            else
                mTimelineModel.addFeed(std::move(feed));

            emit timelineModelChanged();
            setGetTimelineInProgress(false);
       },
       [this](const QString& error){
            qDebug() << "getTimeline FAILED:" << error;
            setGetTimelineInProgress(false);
        }
    );
    // TODO: show error in GUI
}

void Skywalker::getTimelineNextPage()
{
    const QString& cursor = mTimelineModel.getLastCursor();
    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getTimeline(cursor);
}

void Skywalker::setGetTimelineInProgress(bool inProgress)
{
    mGetTimelineInProgress = inProgress;
    emit getTimeLineInProgressChanged();
}

}
