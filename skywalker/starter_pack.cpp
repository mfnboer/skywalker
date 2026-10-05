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

StarterPackViewBasic::StarterPackViewBasic(const QString& uri, const QString& cid, const QString& name,
                     const BasicProfile& creator, const QString& description,
                     const NamedLink::List& embeddedLinks) :
    mUri(uri),
    mCid(cid),
    mName(name),
    mCreator(creator),
    mDescription(description),
    mEmbeddedLinksDescription(embeddedLinks)
{
}

QString StarterPackViewBasic::getUri() const
{
    if (mUri)
        return *mUri;

    if (mBasicView)
        return mBasicView->mUri;

    if (mView)
        return mView->mUri;

    return {};
}

QString StarterPackViewBasic::getCid() const
{
    if (mCid)
        return *mCid;

    if (mBasicView)
        return mBasicView->mCid;

    if (mView)
        return mView->mCid;

    return {};
}

BasicProfile StarterPackViewBasic::getCreator() const
{
    if (mCreator)
        return *mCreator;

    if (mBasicView)
        return BasicProfile(mBasicView->mCreator);

    if (mView)
        return BasicProfile(mView->mCreator);

    return {};
}

QString StarterPackViewBasic::getName() const
{
    if (mName)
        return *mName;

    const auto* starterPack = getStarterPack();
    return starterPack ? starterPack->mName : QString();
}

QString StarterPackViewBasic::getDescription() const
{
    if (mDescription)
        return *mDescription;

    const auto* starterPack = getStarterPack();
    return starterPack ? starterPack->mDescription.value_or("") : QString();
}

QString StarterPackViewBasic::getFormattedDescription() const
{
    if (mDescription)
    {
        const auto facets = NamedLink::toFacetList(mEmbeddedLinksDescription);
        return ATProto::RichTextMaster::linkiFy(*mDescription, facets, UserSettings::getCurrentLinkColor());
    }

    const auto* starterPack = getStarterPack();

    if (!starterPack)
        return {};

    return ATProto::RichTextMaster::getFormattedStarterPackDescription(*starterPack, UserSettings::getCurrentLinkColor());
}

NamedLink::List StarterPackViewBasic::getEmbeddedLinksDescription() const
{
    if (mDescription)
        return mEmbeddedLinksDescription;

    const auto* starterPack = getStarterPack();

    if (!starterPack)
        return {};

    const auto facets = ATProto::RichTextMaster::getEmbeddedLinks(*starterPack->mDescription, starterPack->mDescriptionFacets);
    return NamedLink::fromFacetList(facets);
}

ContentLabelList StarterPackViewBasic::getContentLabels() const
{
    if (mBasicView)
        return ContentFilter::getContentLabels(mBasicView->mLabels);

    if (mView)
        return ContentFilter::getContentLabels(mView->mLabels);

    return {};
}

int StarterPackViewBasic::getListItemCount() const
{
    if (mBasicView)
        return mBasicView->mListItemCount.value_or(-1);

    if (mView && mView->mList)
        return mView->mList->mListItemCount.value_or(-1);

    return -1;
}

const ATProto::AppBskyGraph::StarterPack* StarterPackViewBasic::getStarterPack() const
{
    try {
        if (mBasicView)
            return std::get<ATProto::AppBskyGraph::StarterPack::SharedPtr>(mBasicView->mRecord).get();

        if (mView)
            return std::get<ATProto::AppBskyGraph::StarterPack::SharedPtr>(mView->mRecord).get();
    } catch (const std::bad_variant_access&) {
        qWarning() << "Unknown record type";
        return nullptr;
    }

    return nullptr;
}

void StarterPackViewBasic::setDescription(const QString& description, const NamedLink::List& embeddedLinks)
{
    mDescription = description;
    mEmbeddedLinksDescription = embeddedLinks;
}


StarterPackView::StarterPackView(const ATProto::AppBskyGraph::StarterPackView::SharedPtr& view) :
    StarterPackViewBasic(view)
{
}

StarterPackView::StarterPackView(const ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr& view) :
    StarterPackViewBasic(view)
{
}

StarterPackView::StarterPackView(const QString& uri, const QString& cid, const QString& name,
                                 const BasicProfile& creator, const QString& description,
                                 const NamedLink::List& embeddedLinks) :
    StarterPackViewBasic(uri, cid, name, creator, description, embeddedLinks)
{
}


ListViewBasic StarterPackView::getList() const
{
    return mView ? ListViewBasic(mView->mList) : ListViewBasic();
}

GeneratorView::List StarterPackView::getFeeds() const
{
    if (!mView)
        return {};

    GeneratorView::List feedList;

    for (const auto& feed : mView->mFeeds)
        feedList.emplaceBack(feed);

    return feedList;
}

}
