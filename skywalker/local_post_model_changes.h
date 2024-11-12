// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "list_view_include.h"
#include "record_view.h"
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
        bool mLikeTransient = false;

        int mReplyCountDelta = 0;
        int mRepostCountDelta = 0;
        int mQuoteCountDelta = 0;
        std::optional<QString> mRepostUri;
        std::optional<QString> mThreadgateUri;
        QEnums::ReplyRestriction mReplyRestriction = QEnums::REPLY_RESTRICTION_UNKNOWN;
        std::optional<ListViewBasicList> mReplyRestrictionLists;
        std::optional<QStringList> mHiddenReplies;
        std::optional<bool> mThreadMuted;
        RecordView::SharedPtr mDetachedRecord;
        RecordView::SharedPtr mReAttachedRecord;
        std::optional<bool> mViewerStatePinned;

        bool mPostDeleted = false;
    };

    LocalPostModelChanges() = default;
    virtual ~LocalPostModelChanges() = default;

    const Change* getLocalChange(const QString& cid) const;
    const Change* getLocalUriChange(const QString& uri) const;
    void clearLocalChanges();

    void updatePostIndexedSecondsAgo();
    void updateReplyCountDelta(const QString& cid, int delta);
    void updateRepostCountDelta(const QString& cid, int delta);
    void updateQuoteCountDelta(const QString& cid, int delta);
    void updateRepostUri(const QString& cid, const QString& repostUri);
    void updateLikeCountDelta(const QString& cid, int delta);
    void updateLikeUri(const QString& cid, const QString& likeUri);
    void updateLikeTransient(const QString& cid, bool transient);
    void updateThreadgateUri(const QString& cid, const QString& threadgateUri);
    void updateReplyRestriction(const QString& cid, const QEnums::ReplyRestriction replyRestricion);
    void updateReplyRestrictionLists(const QString& cid, const ListViewBasicList replyRestrictionLists);
    void updateHiddenReplies(const QString& cid, const QStringList& hiddenReplies);
    void updateThreadMuted(const QString& uri, bool muted);

    /**
     * @brief updateDetachedRecord
     * @param cid cid of the embedding post
     * @param postUri the quote uri to be detached, empty uri means re-attach
     * @return true when a re-attached quote should be loaded
     */
    bool updateDetachedRecord(const QString& cid, const QString& postUri);

    void updateReAttachedRecord(const QString& cid, RecordView::SharedPtr record);
    void updateViewerStatePinned(const QString& cid, bool pinned);
    void updatePostDeleted(const QString& cid);

protected:
    virtual void postIndexedSecondsAgoChanged() = 0;
    virtual void likeCountChanged() = 0;
    virtual void likeUriChanged() = 0;
    virtual void likeTransientChanged() = 0;
    virtual void replyCountChanged() = 0;
    virtual void repostCountChanged() = 0;
    virtual void quoteCountChanged() = 0;
    virtual void repostUriChanged() = 0;
    virtual void threadgateUriChanged() = 0;
    virtual void replyRestrictionChanged() = 0;
    virtual void replyRestrictionListsChanged() = 0;
    virtual void hiddenRepliesChanged() = 0;
    virtual void threadMutedChanged() = 0;
    virtual void detachedRecordChanged() = 0;
    virtual void reAttachedRecordChanged() = 0;
    virtual void viewerStatePinnedChanged() = 0;
    virtual void postDeletedChanged() = 0;

private:
    // Mapping from post CID to change
    std::unordered_map<QString, Change> mChanges;

    // Mapping from post URI to change
    std::unordered_map<QString, Change> mUriChanges;
};

}
