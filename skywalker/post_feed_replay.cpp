// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "post_feed_replay.h"
#include "post_feed_model.h"

namespace Skywalker {

void PostFeedReplay::clear()
{
    qDebug() << "Clear";
    mPagePrependSequence.clear();
}

void PostFeedReplay::append(ATProto::AppBskyFeed::OutputFeed::SharedPtr page)
{
    qDebug() << "Append page, size:" << page->mFeed.size() << "cursor:" << page->mCursor.value_or("");

    if (mPagePrependSequence.empty())
        mPagePrependSequence.push_back({});

    auto& pageList = mPagePrependSequence.front();
    pageList.mPages.push_back(page);
}

void PostFeedReplay::prepend(ATProto::AppBskyFeed::OutputFeed::SharedPtr page, bool gapTillNextPage)
{
    qDebug() << "Prepend page, size:" << page->mFeed.size() << "cursor:" << page->mCursor.value_or("") << "gapTillNextPage:" << gapTillNextPage;
    PageList pageList;
    pageList.mPages.push_back(page);
    pageList.mGapTillNextPage = gapTillNextPage;
    mPagePrependSequence.push_back(std::move(pageList));
}

void PostFeedReplay::gapFill(ATProto::AppBskyFeed::OutputFeed::SharedPtr page, const QString& cursor, bool gapTillNextPage)
{
    qDebug() << "Gap fill, size:" << page->mFeed.size() << "cursor:" << page->mCursor.value_or("") << "gapCursor:" << cursor << "gapTillNextPage:" << gapTillNextPage;
    auto* pageList = findGap(cursor);

    Q_ASSERT(pageList);
    if (!pageList)
    {
        qWarning() << "Cannot find gap:" << cursor;
        return;
    }

    Q_ASSERT(pageList->mGapTillNextPage);
    pageList->mPages.push_back(page);
    pageList->mGapTillNextPage = gapTillNextPage;
}

void PostFeedReplay::closeGap(const QString& cursor)
{
    qDebug() << "Close gap, gapCursor:" << cursor;
    auto* pageList = findGap(cursor);

    Q_ASSERT(pageList);
    if (!pageList)
    {
        qWarning() << "Cannot find gap:" << cursor;
        return;
    }

    Q_ASSERT(pageList->mGapTillNextPage);
    pageList->mGapTillNextPage = false;
}

void PostFeedReplay::removeTail(const QString& cursor)
{
    qDebug() << "Remove tail, cursor:" << cursor;
    int i = 0;

    for (; i < (int)mPagePrependSequence.size(); ++i)
    {
        auto& pageList = mPagePrependSequence[i];
        int j = 0;

        for (; j < (int)pageList.mPages.size(); ++j)
        {
            auto& page = pageList.mPages[j];

            if (page->mCursor && *page->mCursor == cursor)
                break;
        }

        if (j < (int)pageList.mPages.size())
        {
            const int newSize = j + 1;
            pageList.mPages.resize(newSize);
            pageList.mGapTillNextPage = false;
            break;
        }
    }

    if (i < (int)mPagePrependSequence.size())
        mPagePrependSequence.erase(mPagePrependSequence.begin(), mPagePrependSequence.begin() + i);
    else
        qWarning() << "Cursor not found:" << cursor;
}

void PostFeedReplay::removeHead(const QString& cursor)
{
    qDebug() << "Remove head, cursor:" << cursor;
    int i = 0;

    for (; i < (int)mPagePrependSequence.size(); ++i)
    {
        auto& pageList = mPagePrependSequence[i];
        int j = 0;

        for (; j < (int)pageList.mPages.size(); ++j)
        {
            auto& page = pageList.mPages[j];

            if (page->mCursor && *page->mCursor == cursor)
                break;
        }

        if (j < (int)pageList.mPages.size())
        {
            pageList.mPages.erase(pageList.mPages.begin(), pageList.mPages.begin() + j);
            break;
        }
    }

    if (i < (int)mPagePrependSequence.size())
        mPagePrependSequence.erase(mPagePrependSequence.begin() + i + 1, mPagePrependSequence.end());
    else
        qWarning() << "Cursor not found:" << cursor;
}

void PostFeedReplay::replay(PostFeedModel& model) const
{
    qDebug() << "Replay";
    model.clear();

    if (mPagePrependSequence.empty())
    {
        qDebug() << "Nothing to replay";
        return;
    }

    const auto& firstPageList = mPagePrependSequence.front();

    for (const auto& page : firstPageList.mPages)
    {
        auto feed = page;
        model.addFeed(std::move(feed));
    }

    for (int i = 1; i < (int)mPagePrependSequence.size(); ++i)
    {
        const auto& pageList = mPagePrependSequence[i];

        if (pageList.mPages.empty())
        {
            qWarning() << "Empty page list:" << i;
            continue;
        }

        auto feed = pageList.mPages.front();
        int gapId = model.prependFeed(std::move(feed));

        for (int j = 1; j < (int)pageList.mPages.size(); ++j)
        {
            if (gapId == 0)
            {
                qWarning() << "No gap to fill:" << j;
                break;
            }

            feed = pageList.mPages[j];
            gapId = model.gapFillFeed(std::move(feed), gapId);
        }

        if (gapId > 0 && !pageList.mGapTillNextPage)
        {
            // Close gap with empty feed page
            feed = std::make_shared<ATProto::AppBskyFeed::OutputFeed>();
            model.gapFillFeed(std::move(feed), gapId);
        }
    }
}

PostFeedReplay::PageList* PostFeedReplay::findGap(const QString& cursor)
{
    for (auto& pageList : mPagePrependSequence)
    {
        Q_ASSERT(!pageList.mPages.empty());
        auto& lastPage = pageList.mPages.back();

        if (lastPage->mCursor && *lastPage->mCursor == cursor)
            return &pageList;
    }

    qDebug() << "Gap not found:" << cursor;
    return nullptr;
}

}
