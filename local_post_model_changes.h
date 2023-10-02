// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QHashFunctions>
#include <QString>
#include <optional>
#include <unordered_map>

namespace Skywalker {

class  LocalPostModelChanges
{
public:
    struct Change
    {
        int mLikeCountDelta = 0;

        // Not-set means not changed.
        // Empty means like removed.
        std::optional<QString> mLikeUri;

        int mReplyCountDelta = 0;
        int mRepostCountDelta = 0;
        std::optional<QString> mRepostUri;
    };

    LocalPostModelChanges() = default;
    virtual ~LocalPostModelChanges() = default;

    const Change* getLocalChange(const QString& cid) const;
    void clearLocalChanges();

    void updatePostIndexTimestamps();
    void updateReplyCountDelta(const QString& cid, int delta);
    void updateRepostCountDelta(const QString& cid, int delta);
    void updateRepostUri(const QString& cid, const QString& repostUri);
    void updateLikeCountDelta(const QString& cid, int delta);
    void updateLikeUri(const QString& cid, const QString& likeUri);

protected:
    virtual void postIndexTimestampChanged() = 0;
    virtual void likeCountChanged() = 0;
    virtual void likeUriChanged() = 0;
    virtual void replyCountChanged() = 0;
    virtual void repostCountChanged() = 0;
    virtual void repostUriChanged() = 0;

private:
    // Mapping from post CID to change
    std::unordered_map<QString, Change> mChanges;
};

}
