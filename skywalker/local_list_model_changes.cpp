// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "local_list_model_changes.h"

namespace Skywalker {

const LocalListModelChanges::Change* LocalListModelChanges::getLocalChange(const QString& uri) const
{
    auto it = mChanges.find(uri);
    return it != mChanges.end() ? &it->second : nullptr;
}

void LocalListModelChanges::clearLocalChanges()
{
    mChanges.clear();
}

void LocalListModelChanges::updateBlocked(const QString& uri, const QString& blockUri)
{
    mChanges[uri].mBlocked = blockUri;
    blockedChanged();
}

void LocalListModelChanges::updateMuted(const QString& uri, bool muted)
{
    mChanges[uri].mMuted = muted;
    mutedChanged();
}

void LocalListModelChanges::hideFromTimeline(const QString& uri, bool hide)
{
    mChanges[uri].mHideFromTimeline = hide;
    hideFromTimelineChanged();
}

void LocalListModelChanges::updateMemberListItemUri(const QString& uri, const QString& listItemUri)
{
    mChanges[uri].mMemberListItemUri = listItemUri;
    memberListItemUriChanged();
}

}
