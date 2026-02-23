// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once

#include "presence.h"
#include <atproto/lib/client.h>

namespace Skywalker {

// Drafts stored on Bluesky have their media files still stored locally.
// If a draft created on device A gets deleted by device B, then the media files
// still exists on device A. This checker cross checks the draft media files
// against Bluesky drafts. Orphaned files will be deleted.
class DraftOrphanedMediaChecker : public Presence
{
public:
    explicit DraftOrphanedMediaChecker(const QString& userDid, ATProto::Client::SharedPtr& bsky);
    ~DraftOrphanedMediaChecker();

    void start(const std::function<void()>& finishedCb);

private:
    void checkMediaFiles(int maxPages = 100, const QString& cursor = {});
    void updateMediaFiles(const ATProto::AppBskyDraft::DraftView::List& drafts);
    void updateMediaFiles(const ATProto::AppBskyDraft::DraftPost::SharedPtr& draftPost);
    void cleanupOrphans();
    void callFinishCb();

    QString getPictureDraftsPath() const;
    QStringList getMediaFileNames() const;

    QString mUserDid;
    ATProto::Client::SharedPtr& mBsky;
    std::unordered_set<QString> mFileNamesToCheck;
    std::function<void()> mFinishedCb;
};

}
