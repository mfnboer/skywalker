// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "follows_activity_store.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto UPDATE_INTERVAL = 31s;

FollowsActivityStore::FollowsActivityStore(Following& following, QObject* parent) :
    QObject(parent),
    mFollowing(following)
{
    mUnfollowConnection = connect(&mFollowing, &Following::stoppedFollowing, this, [this](const QString& did){ handleUnfollow(did); });
    connect(&mUpdateTimer, &QTimer::timeout, this, [this]{ updateActivities(); });
    mUpdateTimer.start(UPDATE_INTERVAL);
}

FollowsActivityStore::~FollowsActivityStore()
{
    disconnect(mUnfollowConnection);
}

void FollowsActivityStore::clear()
{
    for (auto& [_, status] : mDidStatus)
        status->deleteLater();

    mDidStatus.clear();
    mActiveStatusSet.clear();
}

ActivityStatus* FollowsActivityStore::getActivityStatus(const BasicProfile& author)
{
    if (!author.getViewer().isFollowing())
        return &mNotActiveStatus;

    const auto& did = author.getDid();
    auto it = mDidStatus.find(did);

    if (it != mDidStatus.end())
        return it->second;

    auto* status = new ActivityStatus(did, this);
    mDidStatus.insert({did, status});
    return status;
}

void FollowsActivityStore::reportActivity(const BasicProfile& author, QDateTime timestamp)
{
    if (!author.getViewer().isFollowing())
        return;

    auto* status = getActivityStatus(author);
    mActiveStatusSet.erase(status);
    status->setLastActive(timestamp);

    if (status->isActive())
        mActiveStatusSet.insert(status);
}

std::vector<QString> FollowsActivityStore::getActiveFollowsDids() const
{
    std::vector<QString> dids;
    dids.reserve(mActiveStatusSet.size());

    for (auto it = mActiveStatusSet.rbegin(); it != mActiveStatusSet.rend(); ++it)
        dids.push_back((*it)->getDid());

    return dids;
}

void FollowsActivityStore::updateActivities()
{
    const auto now = QDateTime::currentDateTimeUtc();

    for (auto it = mActiveStatusSet.begin(); it != mActiveStatusSet.end(); )
    {
        auto* status = *it;
        status->updateActive(now);

        if (!status->isActive())
            it = mActiveStatusSet.erase(it);
        else
            break; // the remainder is newer and hence will stay active
    }
}

void FollowsActivityStore::handleUnfollow(const QString& did)
{
    auto it = mDidStatus.find(did);

    if (it == mDidStatus.end())
        return;

    qDebug() << "Delete activity status:" << did;
    ActivityStatus* status = it->second;
    mActiveStatusSet.erase(status);
    status->deleteLater();
    mDidStatus.erase(it);
}

void FollowsActivityStore::pause()
{
    qDebug() << "Pause";
    mUpdateTimer.stop();
}

void FollowsActivityStore::resume()
{
    qDebug() << "Resume";
    mUpdateTimer.start(UPDATE_INTERVAL);
}

}
