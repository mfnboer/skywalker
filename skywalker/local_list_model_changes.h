// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QHashFunctions>
#include <QString>
#include <optional>
#include <unordered_map>

namespace Skywalker {

class  LocalListModelChanges
{
public:
    struct Change
    {
        // Not-set means not changed.
        // Empty means block removed.
        std::optional<QString> mBlocked;

        std::optional<bool> mMuted;
        std::optional<bool> mHideFromTimeline;
        std::optional<QString> mMemberListItemUri;
    };

    LocalListModelChanges() = default;
    virtual ~LocalListModelChanges() = default;

    const Change* getLocalChange(const QString& uri) const;
    void clearLocalChanges();

    void updateBlocked(const QString& uri, const QString& blockUri);
    void updateMuted(const QString& uri, bool muted);
    void hideFromTimeline(const QString& uri, bool hide);
    void syncList(const QString& uri, bool sync);
    void hideReplies(const QString& uri, bool hide);
    void hideFollowing(const QString& uri, bool hide);
    void updateMemberListItemUri(const QString& uri, const QString& listItemUri);

protected:
    virtual void blockedChanged() = 0;
    virtual void mutedChanged() = 0;
    virtual void hideFromTimelineChanged() = 0;
    virtual void syncListChanged() = 0;
    virtual void hideRepliesChanged() = 0;
    virtual void hideFollowingChanged() = 0;
    virtual void memberListItemUriChanged() = 0;

private:
    // URI to change
    std::unordered_map<QString, Change> mChanges;
};

}
