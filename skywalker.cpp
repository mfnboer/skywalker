// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "skywalker.h"

namespace Skywalker {

static constexpr int TIMELINE_ADD_PAGE_SIZE = 50;
static constexpr int TIMELINE_PREPEND_PAGE_SIZE = 20;

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

void Skywalker::getTimeline(int limit, const QString& cursor)
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
    mBsky->getTimeline(limit, cur,
       [this, cursor](auto feed){
            if (cursor.isEmpty())
                mTimelineModel.setFeed(std::move(feed));
            else
                mTimelineModel.addFeed(std::move(feed));

            setGetTimelineInProgress(false);
       },
       [this](const QString& error){
            qDebug() << "getTimeline FAILED:" << error;
            setGetTimelineInProgress(false);
        }
    );
    // TODO: show error in GUI
}

void Skywalker::getTimelinePrepend()
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline prepend";

    if (mGetTimelineInProgress)
    {
        qDebug() << "Get timeline still in progress";
        return;
    }

    setGetTimelineInProgress(true);
    mBsky->getTimeline(TIMELINE_PREPEND_PAGE_SIZE, {},
        [this](auto feed){
            mTimelineModel.prependFeed(std::move(feed));
            setGetTimelineInProgress(false);
        },
        [this](const QString& error){
            qDebug() << "getTimeline FAILED:" << error;
            setGetTimelineInProgress(false);
        }
        );
    // TODO: show error in GUI
}

void Skywalker::getTimelineForGap(size_t gapIndex)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline for gap:" << gapIndex;

    if (mGetTimelineInProgress)
    {
        qDebug() << "Get timeline still in progress";
        return;
    }

    const Post* post = mTimelineModel.getPostAt(gapIndex);
    if (!post || !post->isPlaceHolder())
    {
        qWarning() << "NO GAP AT index:" << gapIndex;
        return;
    }

    std::optional<QString> cur = post->getGapCursor();
    if (!cur || cur->isEmpty())
    {
        qWarning() << "NO CURSOR FOR GAP:" << gapIndex;
        return;
    }

    qDebug() << "Set gap cursor:" << *cur;

    setGetTimelineInProgress(true);
    mBsky->getTimeline(TIMELINE_PREPEND_PAGE_SIZE, cur,
        [this, gapIndex](auto feed){
            mTimelineModel.gapFillFeed(std::move(feed), gapIndex);
            setGetTimelineInProgress(false);
        },
        [this](const QString& error){
            qDebug() << "getTimelineForGap FAILED:" << error;
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

    getTimeline(TIMELINE_ADD_PAGE_SIZE, cursor);
}

void Skywalker::setGetTimelineInProgress(bool inProgress)
{
    mGetTimelineInProgress = inProgress;
    emit getTimeLineInProgressChanged();
}

}
