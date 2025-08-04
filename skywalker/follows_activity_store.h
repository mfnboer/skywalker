// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "activity_status.h"
#include "profile_store.h"
#include <QObject>

namespace Skywalker {

class FollowsActivityStore : public QObject
{
    Q_OBJECT

public:
    explicit FollowsActivityStore(const IProfileStore& userFollows, QObject* parent = nullptr);

    void clear();
    Q_INVOKABLE ActivityStatus* getActivityStatus(const QString& did);
    void reportActivity(const QString& did, QDateTime timestamp);

private:
    const IProfileStore& mUserFollows;
    ActivityStatus mNotActiveStatus{this};
    std::unordered_map<QString, ActivityStatus*> mDidStatus;
};

}
