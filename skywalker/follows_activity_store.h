// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "activity_status.h"
#include "profile_store.h"
#include <QTimer>
#include <QObject>

namespace Skywalker {

class FollowsActivityStore : public QObject
{
    Q_OBJECT

public:
    explicit FollowsActivityStore(IProfileStore& userFollows, QObject* parent = nullptr);

    void clear();
    Q_INVOKABLE ActivityStatus* getActivityStatus(const QString& did);
    void reportActivity(const QString& did, QDateTime timestamp);

    // From newest to oldest activity
    std::vector<QString> getActiveFollowsDids() const;

    void pause();
    void resume();

private:
    void updateActivities();
    void handleUnfollow(const QString& did);

    IProfileStore& mUserFollows;
    ActivityStatus mNotActiveStatus{"", this};
    std::unordered_map<QString, ActivityStatus*> mDidStatus;

    // From oldest to newest activity
    std::set<ActivityStatus*, ActiviyStatusPtrCmp> mActiveStatusSet;

    QTimer mUpdateTimer;
    ScopedHandle* mRemovedCbHandle = nullptr;
};

}
