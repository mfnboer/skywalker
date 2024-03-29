// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile.h"
#include "content_filter.h"
#include "definitions.h"

namespace Skywalker {

ProfileViewerState::ProfileViewerState(const ATProto::AppBskyActor::ViewerState& viewerState) :
    mValid(true),
    mMuted(viewerState.mMuted),
    mBlockedBy(viewerState.mBlockedBy),
    mBlocking(viewerState.mBlocking.value_or("")),
    mFollowing(viewerState.mFollowing.value_or("")),
    mFollowedBy(viewerState.mFollowedBy.value_or(""))
{
    if (viewerState.mMutedByList)
    {
        const auto& l = *viewerState.mMutedByList;
        mMutedByList = ListViewBasic(l.mUri, l.mCid, l.mName, l.mPurpose,
                                      l.mAvatar.value_or(""));
    }

    if (viewerState.mBlockingByList)
    {
        const auto& l = *viewerState.mBlockingByList;
        mBlockingByList = ListViewBasic(l.mUri, l.mCid, l.mName, l.mPurpose,
                                        l.mAvatar.value_or(""));
    }
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic* profile) :
    mProfileBasicView(profile)
{
    Q_ASSERT(mProfileBasicView);
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileView* profile) :
    mProfileView(profile)
{
    Q_ASSERT(mProfileView);
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileViewDetailed* profile) :
    mProfileDetailedView(profile)
{
    Q_ASSERT(mProfileDetailedView);
}

BasicProfile::BasicProfile(const QString& did, const QString& handle, const QString& displayName,
                           const QString& avatarUrl, const ProfileViewerState& viewer,
                           const ContentLabelList& contentLabels) :
    mDid(did),
    mHandle(handle),
    mDisplayName(displayName),
    mAvatarUrl(avatarUrl),
    mViewer(viewer),
    mContentLabels(contentLabels)
{
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileView& profile) :
    mDid(profile.mDid),
    mHandle(profile.mHandle),
    mDisplayName(profile.mDisplayName.value_or(QString())),
    mAvatarUrl(profile.mAvatar.value_or(QString()))
{
}

bool BasicProfile::isNull() const
{
    return getDid().isEmpty();
}

QString BasicProfile::getDid() const
{
    if (mProfileBasicView)
        return mProfileBasicView->mDid;

    if (mProfileView)
        return mProfileView->mDid;

    if (mProfileDetailedView)
        return mProfileDetailedView->mDid;

    return mDid;
}

static QString createName(const QString& handle, const QString& displayName)
{
    const QString name = displayName.trimmed();
    return name.isEmpty() ? handle : name;
}

QString BasicProfile::getName() const
{
    return createName(getHandle(), getDisplayName());
}

QString BasicProfile::getDisplayName() const
{
    if (mProfileBasicView)
        return mProfileBasicView->mDisplayName.value_or("");

    if (mProfileView)
        return mProfileView->mDisplayName.value_or("");

    if (mProfileDetailedView)
        return mProfileDetailedView->mDisplayName.value_or("");

    return mDisplayName;
}

QString BasicProfile::getHandle() const
{
    if (mProfileBasicView)
        return mProfileBasicView->mHandle;

    if (mProfileView)
        return mProfileView->mHandle;

    if (mProfileDetailedView)
        return mProfileDetailedView->mHandle;

    return mHandle;
}

bool BasicProfile::hasInvalidHandle() const
{
    return getHandle() == INVALID_HANDLE;
}

QString BasicProfile::getHandleOrDid() const
{
    const QString& handle = getHandle();

    if (handle.isEmpty() || handle == INVALID_HANDLE)
        return getDid();

    return handle;
}

QString BasicProfile::getAvatarUrl() const
{
    if (mProfileBasicView)
        return mProfileBasicView->mAvatar.value_or("");

    if (mProfileView)
        return mProfileView->mAvatar.value_or("");

    if (mProfileDetailedView)
        return mProfileDetailedView->mAvatar.value_or("");

    return mAvatarUrl;
}

ImageView BasicProfile::getImageView() const
{
    return ImageView(getAvatarUrl(), getName());
}

ProfileViewerState BasicProfile::getViewer() const
{
    if (mProfileBasicView)
        return mProfileBasicView->mViewer ? ProfileViewerState(*mProfileBasicView->mViewer) : ProfileViewerState{};

    if (mProfileView)
        return mProfileView->mViewer ? ProfileViewerState(*mProfileView->mViewer) : ProfileViewerState{};

    if (mProfileDetailedView)
        return mProfileDetailedView->mViewer ? ProfileViewerState(*mProfileDetailedView->mViewer) : ProfileViewerState{};

    return mViewer;
}

ContentLabelList BasicProfile::getContentLabels() const
{
    const ContentFilter::LabelList* labels = nullptr;

    if (mProfileBasicView)
        labels = &mProfileBasicView->mLabels;
    else if (mProfileView)
        labels = &mProfileView->mLabels;
    else if (mProfileDetailedView)
        labels = &mProfileDetailedView->mLabels;

    if (!labels)
        return mContentLabels;

    return ContentFilter::getContentLabels(*labels);
}

bool BasicProfile::isVolatile() const
{
    return mProfileBasicView || mProfileView || mProfileDetailedView;
}

BasicProfile BasicProfile::nonVolatileCopy() const
{
    BasicProfile profile(getDid(), getHandle(), getDisplayName(),
                         getAvatarUrl(), getViewer(), getContentLabels());
    return profile;
}

void BasicProfile::setAvatarUrl(const QString& avatarUrl)
{
    mAvatarUrl = avatarUrl;

    if (avatarUrl.startsWith("image://"))
    {
        auto* provider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
        mAvatarSource = std::make_shared<SharedImageSource>(avatarUrl, provider);
    }
    else
    {
        mAvatarSource = nullptr;
    }
}

Profile::Profile(const ATProto::AppBskyActor::ProfileView* profile) :
    BasicProfile(profile)
{
}

Profile::Profile(const ATProto::AppBskyActor::ProfileViewDetailed* profile) :
    BasicProfile(profile)
{
}

Profile::Profile(const ATProto::AppBskyActor::ProfileView::SharedPtr& profile) :
    BasicProfile(profile.get()),
    mProfile(profile)
{
}

Profile::Profile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile) :
    BasicProfile(profile.get()),
    mDetailedProfile(profile)
{
}

Profile::Profile(const QString& did, const QString& handle, const QString& displayName,
                 const QString& avatarUrl, const ProfileViewerState& viewer,
                 const ContentLabelList& contentLabels, const QString& description) :
    BasicProfile(did, handle, displayName, avatarUrl, viewer, contentLabels),
    mDescription(description)
{
}

QString Profile::getDescription() const
{
    if (mProfileView)
        return mProfileView->mDescription.value_or("");

    if (mProfileDetailedView)
        return mProfileDetailedView->mDescription.value_or("");

    return mDescription;
}

Profile Profile::nonVolatileCopy() const
{
    Profile profile(getDid(), getHandle(), getDisplayName(), getAvatarUrl(),
                    getViewer(), getContentLabels(), getDescription());
    return profile;
}

DetailedProfile::DetailedProfile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile) :
    Profile(profile.get()),
    mDetailedProfile(profile)
{
}

DetailedProfile:: DetailedProfile(const QString& did, const QString& handle, const QString& displayName,
                const QString& avatarUrl, const ProfileViewerState& viewer,
                const ContentLabelList& contentLabels, const QString& description,
                const QString& banner, int followersCount, int followsCount, int postsCount) :
    Profile(did, handle, displayName, avatarUrl, viewer, contentLabels, description),
    mBanner(banner),
    mFollowersCount(followersCount),
    mFollowsCount(followsCount),
    mPostsCount(postsCount)
{
}

QString DetailedProfile::getBanner() const
{
    return mProfileDetailedView ? mProfileDetailedView->mBanner.value_or("") : mBanner;
}

int DetailedProfile::getFollowersCount() const
{
    return mProfileDetailedView ? mProfileDetailedView->mFollowersCount : mFollowersCount;
}

int DetailedProfile::getFollowsCount() const
{
    return mProfileDetailedView ? mProfileDetailedView->mFollowsCount : mFollowsCount;
}

int DetailedProfile::getPostsCount() const
{
    return mProfileDetailedView ? mProfileDetailedView->mPostsCount : mPostsCount;
}

DetailedProfile DetailedProfile::nonVolatileCopy() const
{
    DetailedProfile profile(getDid(), getHandle(), getDisplayName(), getAvatarUrl(),
                            getViewer(), getContentLabels(), getDescription(), getBanner(),
                            getFollowersCount(), getFollowsCount(), getPostsCount());
    return profile;
}

}
