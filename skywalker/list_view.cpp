// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "list_view.h"
#include "content_filter.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

ListViewerState::ListViewerState(const ATProto::AppBskyGraph::ListViewerState::SharedPtr& viewerState) :
    mViewerState(viewerState)
{
}

bool ListViewerState::getMuted() const
{
    return mViewerState ? mViewerState->mMuted : false;
}

QString ListViewerState::getBlocked() const
{
    return mViewerState ? mViewerState->mBlocked.value_or("") : "";
}

ListViewBasic::ListViewBasic(const ATProto::AppBskyGraph::ListViewBasic::SharedPtr& view) :
    mListViewBasic(view)
{
}

ListViewBasic::ListViewBasic(const ATProto::AppBskyGraph::ListView::SharedPtr& view) :
    mListView(view)
{
}

ListViewBasic::ListViewBasic(const QString& uri, const QString& cid, const QString& name,
              ATProto::AppBskyGraph::ListPurpose purpose, const QString& avatar) :
    mUri(uri),
    mCid(cid),
    mName(name),
    mPurpose(purpose),
    mAvatar(avatar)
{
}

QString ListViewBasic::getUri() const
{
    if (mUri)
        return *mUri;

    if (mListViewBasic)
        return mListViewBasic->mUri;

    if (mListView)
        return mListView->mUri;

    return {};
}

QString ListViewBasic::getCid() const
{
    if (mCid)
        return *mCid;

    if (mListViewBasic)
        return mListViewBasic->mCid;

    if (mListView)
        return mListView->mCid;

    return {};
}

QString ListViewBasic::getName() const
{
    if (mName)
        return *mName;

    if (mListViewBasic)
        return mListViewBasic->mName;

    if (mListView)
        return mListView->mName;

    return {};
}

QEnums::ListPurpose ListViewBasic::getPurpose() const
{
    if (mPurpose != ATProto::AppBskyGraph::ListPurpose::UNKNOWN)
        return QEnums::ListPurpose(mPurpose);

    if (mListViewBasic)
        return QEnums::ListPurpose(mListViewBasic->mPurpose);

    if (mListView)
        return QEnums::ListPurpose(mListView->mPurpose);

    return QEnums::LIST_PURPOSE_UNKNOWN;
}

QString ListViewBasic::getAvatar() const
{
    if (mAvatar)
        return *mAvatar;

    if (mListViewBasic)
        return mListViewBasic->mAvatar.value_or("");

    if (mListView)
        return mListView->mAvatar.value_or("");

    return {};
}

QString ListViewBasic::getAvatarThumb() const
{
    return ATProto::createAvatarThumbUrl(getAvatar());
}

ImageView ListViewBasic::getImageView() const
{
    return ImageView(getAvatar(), getName());
}

ContentLabelList ListViewBasic::getContentLabels() const
{
    if (mListViewBasic)
        return ContentFilter::getContentLabels(mListViewBasic->mLabels);

    if (mListView)
        return ContentFilter::getContentLabels(mListView->mLabels);

    return {};
}

ListViewerState ListViewBasic::getViewer() const
{
    if (mViewer)
        return *mViewer;

    if (mListViewBasic)
        return mListViewBasic->mViewer ? ListViewerState(mListViewBasic->mViewer) : ListViewerState{};

    if (mListView)
        return mListView->mViewer ? ListViewerState(mListView->mViewer) : ListViewerState{};

    return {};
}

void ListViewBasic::setAvatar(const QString& avatar)
{
    mAvatar = avatar;

    if (avatar.startsWith("image://"))
    {
        auto* provider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
        mAvatarSource = std::make_shared<SharedImageSource>(avatar, provider);
    }
    else
    {
        mAvatarSource = nullptr;
    }
}


ListView::ListView(const ATProto::AppBskyGraph::ListView::SharedPtr& view) :
    ListViewBasic(view)
{
}

ListView::ListView(const QString& uri, const QString& cid, const QString& name,
         ATProto::AppBskyGraph::ListPurpose purpose, const QString& avatar,
         const Profile& creator, const QString& description,
         const WebLink::List& embeddedLinks) :
    ListViewBasic(uri, cid, name, purpose, avatar),
    mCreator(creator),
    mDescription(description),
    mEmbeddedLinksDescription(embeddedLinks)
{
}

Profile ListView::getCreator() const
{
    if (mCreator)
        return *mCreator;

    return mListView ? Profile(mListView->mCreator) : Profile{};
}

QString ListView::getDescription() const
{
    if (mDescription)
        return *mDescription;

    if (!mListView || !mListView->mDescription)
        return {};

    return *mListView->mDescription;
}

QString ListView::getFormattedDescription() const
{
    if (mDescription)
    {
        const auto facets = WebLink::toFacetList(mEmbeddedLinksDescription);
        return ATProto::RichTextMaster::linkiFy(*mDescription, facets, UserSettings::getCurrentLinkColor());
    }

    if (!mListView || !mListView->mDescription)
        return {};

    return ATProto::RichTextMaster::getFormattedListDescription(*mListView, UserSettings::getCurrentLinkColor());
}

WebLink::List ListView::getEmbeddedLinksDescription() const
{
    if (mDescription)
        return mEmbeddedLinksDescription;

    if (!mListView || !mListView->mDescription)
        return {};

    const auto facets = ATProto::RichTextMaster::getEmbeddedLinks(*mListView->mDescription, mListView->mDescriptionFacets);
    return WebLink::fromFacetList(facets);
}

void ListView::setDescription(const QString& description, const WebLink::List& embeddedLinks)
{
    mDescription = description;
    mEmbeddedLinksDescription = embeddedLinks;
}

}
