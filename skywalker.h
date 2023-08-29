// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post_feed_model.h"
#include <atproto/lib/client.h>
#include <QObject>
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
    Q_INVOKABLE void getTimeline(int limit, const QString& cursor = {});
    Q_INVOKABLE void getTimelinePrepend();
    Q_INVOKABLE void getTimelineForGap(size_t gapIndex);
    Q_INVOKABLE void getTimelineNextPage();

    const PostFeedModel* getTimelineModel() const { return &mTimelineModel; }
    void setGetTimelineInProgress(bool inProgress);
    bool isGetTimelineInProgress() const { return mGetTimelineInProgress; }

signals:
    void loginOk();
    void loginFailed(QString error);
    void getTimeLineInProgressChanged();

private:
    std::unique_ptr<ATProto::Client> mBsky;
    PostFeedModel mTimelineModel;
    bool mGetTimelineInProgress = false;
};

}
