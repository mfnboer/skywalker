// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "list_view.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

ListViewerState::ListViewerState(const ATProto::AppBskyGraph::ListViewerState& viewerState) :
    mMuted(viewerState.mMuted),
    mBlocked(viewerState.mBlocked.value_or(""))
{
}

ListViewBasic::ListViewBasic(const ATProto::AppBskyGraph::ListViewBasic::SharedPtr& view) :
    mSharedListViewBasic(view)
{
    Q_ASSERT(mSharedListViewBasic);
}

ListViewBasic::ListViewBasic(const ATProto::AppBskyGraph::ListView* view) :
    mRawListView(view)
{
    Q_ASSERT(mRawListView);
}

ListViewBasic::ListViewBasic(const ATProto::AppBskyGraph::ListViewBasic* view) :
    mRawListViewBasic(view)
{
    Q_ASSERT(mRawListViewBasic);
}

QString ListViewBasic::getUri() const
{
    if (basicView())
        return basicView()->mUri;

    if (view())
        return view()->mUri;

    return {};
}

QString ListViewBasic::getCid() const
{
    if (!mCid.isEmpty())
        return mCid;

    if (basicView())
        return basicView()->mCid;

    if (view())
        return view()->mCid;

    return {};
}

QString ListViewBasic::getName() const
{
    if (!mName.isEmpty())
        return mName;

    if (basicView())
        return basicView()->mName;

    if (view())
        return view()->mName;

    return {};
}

QEnums::ListPurpose ListViewBasic::getPurpose() const
{
    if (basicView())
        return QEnums::ListPurpose(basicView()->mPurpose);

    if (view())
        return QEnums::ListPurpose(view()->mPurpose);

    return QEnums::LIST_PURPOSE_UNKNOWN;
}

QString ListViewBasic::getAvatar() const
{
    if (mAvatar)
        return *mAvatar;

    if (basicView())
        return basicView()->mAvatar.value_or("");

    if (view())
        return view()->mAvatar.value_or("");

    return {};
}

ImageView ListViewBasic::getImageView() const
{
    return ImageView(getAvatar(), getName());
}

ListViewerState ListViewBasic::getViewer() const
{
    if (basicView())
        return basicView()->mViewer ? ListViewerState(*basicView()->mViewer) : ListViewerState{};

    if (view())
        return view()->mViewer ? ListViewerState(*view()->mViewer) : ListViewerState{};

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

const ATProto::AppBskyGraph::ListView* ListViewBasic::view() const
{
    return mRawListView;
}

const ATProto::AppBskyGraph::ListViewBasic* ListViewBasic::basicView() const
{
    return mSharedListViewBasic ? mSharedListViewBasic.get() : mRawListViewBasic;
}

ListView::ListView(const ATProto::AppBskyGraph::ListView::SharedPtr& view) :
    ListViewBasic(view.get()),
    mSharedListView(view)
{
}

ListView::ListView(const ATProto::AppBskyGraph::ListView* view) :
    ListViewBasic(view)
{
}

Profile ListView::getCreator() const
{
    return view() ? Profile(view()->mCreator.get()) : Profile{};
}

QString ListView::getDescription() const
{
    if (mDescription)
        return *mDescription;

    if (!view() || !view()->mDescription)
        return {};

    return *view()->mDescription;
}

QString ListView::getFormattedDescription() const
{
    if (mDescription)
        return ATProto::RichTextMaster::linkiFy(*mDescription, UserSettings::getLinkColor());

    if (!view() || !view()->mDescription)
        return {};

    return ATProto::RichTextMaster::getFormattedListDescription(*view(), UserSettings::getLinkColor());
}

}
