// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "starter_pack.h"
#include "content_filter.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

StarterPackViewBasic::StarterPackViewBasic(const ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr& view) :
    mBasicView(view)
{
    Q_ASSERT(mBasicView);
}

StarterPackViewBasic::StarterPackViewBasic(const ATProto::AppBskyGraph::StarterPackView::SharedPtr& view) :
    mView(view)
{
    Q_ASSERT(mView);
}

QString StarterPackViewBasic::getUri() const
{
    if (mBasicView)
        return mBasicView->mUri;

    if (mView)
        return mView->mUri;

    return {};
}

QString StarterPackViewBasic::getCid() const
{
    if (mBasicView)
        return mBasicView->mCid;

    if (mView)
        return mView->mCid;

    return {};
}

BasicProfile StarterPackViewBasic::getCreator() const
{
    if (mBasicView)
        return BasicProfile(mBasicView->mCreator.get()).nonVolatileCopy();

    if (mView)
        return BasicProfile(mView->mCreator.get()).nonVolatileCopy();

    return {};
}

QString StarterPackViewBasic::getName() const
{
    const auto* starterPack = getStarterPack();
    return starterPack ? starterPack->mName : QString();
}

QString StarterPackViewBasic::getDescription() const
{
    const auto* starterPack = getStarterPack();
    return starterPack ? starterPack->mDescription.value_or("") : QString();
}

QString StarterPackViewBasic::getFormattedDescription() const
{
    const auto* starterPack = getStarterPack();

    if (!starterPack)
        return {};

    return ATProto::RichTextMaster::getFormattedStarterPackDescription(*starterPack, UserSettings::getLinkColor());
}

ContentLabelList StarterPackViewBasic::getContentLabels() const
{
    if (mBasicView)
        return ContentFilter::getContentLabels(mBasicView->mLabels);

    if (mView)
        return ContentFilter::getContentLabels(mView->mLabels);

    return {};
}

const ATProto::AppBskyGraph::StarterPack* StarterPackViewBasic::getStarterPack() const
{
    if (mBasicView)
        return std::get<ATProto::AppBskyGraph::StarterPack::Ptr>(mBasicView->mRecord).get();

    if (mView)
        return std::get<ATProto::AppBskyGraph::StarterPack::Ptr>(mView->mRecord).get();

    return nullptr;
}


StarterPackView::StarterPackView(const ATProto::AppBskyGraph::StarterPackView::SharedPtr& view) :
    StarterPackViewBasic(view)
{
}

ListViewBasic StarterPackView::getList() const
{
    return mView ? ListViewBasic(mView->mList.get()).nonVolatileCopy() : ListViewBasic();
}

GeneratorViewList StarterPackView::getFeeds() const
{
    if (!mView)
        return {};

    GeneratorViewList feedList;

    for (const auto& feed : mView->mFeeds)
        feedList.emplaceBack(feed.get());

    return feedList;
}

}
