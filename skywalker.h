// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post_feed_model.h"
#include <atproto/lib/client.h>
#include <QObject>
#include <QTimer>
#include <QtQmlIntegration>

namespace Skywalker {

class Skywalker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(const PostFeedModel* timelineModel READ getTimelineModel CONSTANT FINAL)
    Q_PROPERTY(bool getTimelineInProgress READ isGetTimelineInProgress NOTIFY getTimeLineInProgressChanged FINAL)
    QML_ELEMENT
public:
    explicit Skywalker(QObject* parent = nullptr);

    Q_INVOKABLE void login(const QString user, QString password, const QString host);
    Q_INVOKABLE void resumeSession();
    Q_INVOKABLE void getTimeline(int limit, const QString& cursor = {});
    Q_INVOKABLE void getTimelinePrepend(int autoGapFill = 0);
    Q_INVOKABLE void getTimelineForGap(int gapId, int autoGapFill = 0);
    Q_INVOKABLE void getTimelineNextPage();
    Q_INVOKABLE void timelineMovementEnded(int firstVisibleIndex, int lastVisibleIndex);

    const PostFeedModel* getTimelineModel() const { return &mTimelineModel; }
    void setGetTimelineInProgress(bool inProgress);
    bool isGetTimelineInProgress() const { return mGetTimelineInProgress; }

signals:
    void loginOk();
    void loginFailed(QString error);
    void resumeSessionOk();
    void resumeSessionFailed();
    void getTimeLineInProgressChanged();
    void sessionExpired(QString error);

private:
    void startRefreshTimer();
    void stopRefreshTimer();
    void refreshSession();
    void SaveSession(const QString& host, const ATProto::ComATProtoServer::Session& session);
    bool GetSession(QString& host, ATProto::ComATProtoServer::Session& session);

    std::unique_ptr<ATProto::Client> mBsky;
    PostFeedModel mTimelineModel;
    bool mGetTimelineInProgress = false;
    QTimer mRefreshTimer;
};

}
