// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once

#include "presence.h"
#include <atproto/lib/client.h>

namespace Skywalker {

class DraftOrphanedMediaChecker : public Presence
{
public:
    explicit DraftOrphanedMediaChecker(ATProto::Client::SharedPtr& bsky);
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

    ATProto::Client::SharedPtr& mBsky;
    std::unordered_set<QString> mFileNamesToCheck;
    std::function<void()> mFinishedCb;
};

}
