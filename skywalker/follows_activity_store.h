// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "activity_status.h"
#include "following.h"
#include "profile.h"
#include <QTimer>
#include <QObject>

namespace Skywalker {

class FollowsActivityStore : public QObject
{
    Q_OBJECT

public:
    explicit FollowsActivityStore(Following& following, QObject* parent = nullptr);
    ~FollowsActivityStore();

    void clear();
    Q_INVOKABLE ActivityStatus* getActivityStatus(const BasicProfile& author);
    void reportActivity(const BasicProfile& author, QDateTime timestamp);

    // From newest to oldest activity
    std::vector<QString> getActiveFollowsDids() const;

    void pause();
    void resume();

private:
    void updateActivities();
    void handleUnfollow(const QString& did);

    Following& mFollowing;
    ActivityStatus mNotActiveStatus{"", this};
    std::unordered_map<QString, ActivityStatus*> mDidStatus;

    // From oldest to newest activity
    std::set<ActivityStatus*, ActiviyStatusPtrCmp> mActiveStatusSet;

    QTimer mUpdateTimer;
    QMetaObject::Connection mUnfollowConnection;
};

}
