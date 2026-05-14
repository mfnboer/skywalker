// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>

namespace Skywalker {

class IFeedPager : public QObject
{
    Q_OBJECT

public:
    using updateTimelineCb = std::function<void(bool gapFilled)>;

    explicit IFeedPager(QObject* parent = nullptr) : QObject(parent) {}

    virtual bool isGetTimelineInProgress() const = 0;
    virtual void getTimeline(int limit, int maxPages = 20, int minEntries = 10, const QString& cursor = {}) = 0;
    virtual void getTimelineNextPage(int maxPages = 20, int minEntries = 10) = 0;
    virtual void updateTimeline(int autoGapFill, int pageSize, const updateTimelineCb& cb = {}) = 0;

    virtual void getFeed(int modelId, int limit = 50, int maxPages = 5, int minEntries = 10, const QString& cursor = {}) = 0;
    virtual void getFeedNextPage(int modelId, int maxPages = 5, int minEntries = 10) = 0;
    virtual void getFeedPrepend(int modelId, int autoGapFill = 0, int limit = 50) = 0;
    virtual void getFeedForGap(int modelId, int gapId, int autoGapFill = 0, bool userInitiated = false) = 0;
    virtual bool updateFeed(int modelId, int autoGapFill, int limit) = 0;
    virtual void syncFeed(int modelId, int maxPages = 20) = 0;

    virtual void getListFeed(int modelId, int limit = 50, int maxPages = 5, int minEntries = 10, const QString& cursor = {}) = 0;
    virtual void getListFeedNextPage(int modelId, int maxPages = 5, int minEntries = 10) = 0;
    virtual void getListFeedPrepend(int modelId, int autoGapFill = 0, int limit = 50) = 0;
    virtual void getListFeedForGap(int modelId, int gapId, int autoGapFill = 0, bool userInitiated = false) = 0;
    virtual bool updateListFeed(int modelId, int autoGapFill, int limit) = 0;
    virtual void syncListFeed(int modelId, int maxPages = 20) = 0;

    virtual void getAuthorFeed(int id, int limit = 50, int maxPages = 20, int minEntries = 10, const QString& cursor = {}) = 0;
    virtual void getAuthorFeedNextPage(int id, int maxPages = 20, int minEntries = 10) = 0;
};

}
