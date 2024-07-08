// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile.h"
#include "content_filter.h"
#include "definitions.h"

namespace Skywalker {

KnownFollowers::KnownFollowers(const ATProto::AppBskyActor::KnownFollowers* knownFollowers)
{
    if (!knownFollowers)
        return;

    mCount = knownFollowers->mCount;
    mFollowers.reserve(knownFollowers->mFollowers.size());

    for (const auto& follower : knownFollowers->mFollowers)
    {
        auto profile = std::make_shared<BasicProfile>(follower);
        mFollowers.push_back(profile);

        // Cap followers to the maximum to be safe in case the networks gives
        // much more.
        if (mFollowers.size() >= ATProto::AppBskyActor::KnownFollowers::MAX_COUNT)
            break;
    }
}

QList<BasicProfile> KnownFollowers::getFollowers() const
{
    BasicProfileList followers;
    followers.reserve(mFollowers.size());

    for (const auto& follower : mFollowers)
        followers.emplace_back(*follower);

    return followers;
}

// TODO: store shared pointer?
ProfileViewerState::ProfileViewerState(const ATProto::AppBskyActor::ViewerState& viewerState) :
    mValid(true),
    mMuted(viewerState.mMuted),
    mBlockedBy(viewerState.mBlockedBy),
    mBlocking(viewerState.mBlocking.value_or("")),
    mFollowing(viewerState.mFollowing.value_or("")),
    mFollowedBy(viewerState.mFollowedBy.value_or("")),
    mKnownFollowers(viewerState.mKnownFollowers.get())
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

ProfileAssociatedChat::ProfileAssociatedChat(const ATProto::AppBskyActor::ProfileAssociatedChat::SharedPtr& associated) :
    mAssociated(associated)
{
}

QEnums::AllowIncomingChat ProfileAssociatedChat::getAllowIncoming() const
{
    // Default bsky setting is following
    return mAssociated ? (QEnums::AllowIncomingChat)mAssociated->mAllowIncoming : QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
}


ProfileAssociated::ProfileAssociated(const ATProto::AppBskyActor::ProfileAssociated::SharedPtr& associated) :
    mAssociated(associated)
{
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic::SharedPtr& profile) :
    mProfileBasicView(profile)
{
    Q_ASSERT(mProfileBasicView);
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileView::SharedPtr& profile) :
    mProfileView(profile)
{
    Q_ASSERT(mProfileView);
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile) :
    mProfileDetailedView(profile)
{
    Q_ASSERT(mProfileDetailedView);
}

BasicProfile::BasicProfile(const QString& did, const QString& handle, const QString& displayName,
                           const QString& avatarUrl, const ProfileAssociated& associated,
                           const ProfileViewerState& viewer,
                           const ContentLabelList& contentLabels) :
    mDid(did),
    mHandle(handle),
    mDisplayName(displayName),
    mAvatarUrl(avatarUrl),
    mAssociated(associated),
    mViewer(viewer),
    mContentLabels(contentLabels)
{
}

bool BasicProfile::isNull() const
{
    return getDid().isEmpty();
}

QString BasicProfile::getDid() const
{
    if (mDid)
        return *mDid;

    if (mProfileBasicView)
        return mProfileBasicView->mDid;

    if (mProfileView)
        return mProfileView->mDid;

    if (mProfileDetailedView)
        return mProfileDetailedView->mDid;

    return {};
}

static QString createName(const QString& handle, const QString& displayName)
{
    const QString name = displayName.trimmed();
    return name.isEmpty() ? handle : name;
}

QString BasicProfile::getName() const
{
    const QString name = createName(getHandle(), getDisplayName());

    if (hasInvalidHandle())
        return QString("⚠️ %1").arg(name);

    return name;
}

QString BasicProfile::getDisplayName() const
{
    if (mDisplayName)
        return *mDisplayName;

    if (mProfileBasicView)
        return mProfileBasicView->mDisplayName.value_or("");

    if (mProfileView)
        return mProfileView->mDisplayName.value_or("");

    if (mProfileDetailedView)
        return mProfileDetailedView->mDisplayName.value_or("");

    return {};
}

QString BasicProfile::getHandle() const
{
    if (mHandle)
        return *mHandle;

    if (mProfileBasicView)
        return mProfileBasicView->mHandle;

    if (mProfileView)
        return mProfileView->mHandle;

    if (mProfileDetailedView)
        return mProfileDetailedView->mHandle;

    return {};
}

bool BasicProfile::hasInvalidHandle() const
{
    return getHandle().endsWith(INVALID_HANDLE_SUFFIX);
}

QString BasicProfile::getHandleOrDid() const
{
    const QString& handle = getHandle();

    if (handle.isEmpty() || handle.endsWith(INVALID_HANDLE_SUFFIX))
        return getDid();

    return handle;
}

QString BasicProfile::getAvatarUrl() const
{
    if (mAvatarUrl)
        return *mAvatarUrl;

    if (mProfileBasicView)
        return mProfileBasicView->mAvatar.value_or("");

    if (mProfileView)
        return mProfileView->mAvatar.value_or("");

    if (mProfileDetailedView)
        return mProfileDetailedView->mAvatar.value_or("");

    return {};
}

ImageView BasicProfile::getImageView() const
{
    return ImageView(getAvatarUrl(), getName());
}

ProfileAssociated BasicProfile::getAssociated() const
{
    if (mAssociated)
        return *mAssociated;

    if (mProfileBasicView)
        return mProfileBasicView->mAssociated ? ProfileAssociated(mProfileBasicView->mAssociated) : ProfileAssociated{};

    if (mProfileView)
        return mProfileView->mAssociated ? ProfileAssociated(mProfileView->mAssociated) : ProfileAssociated{};

    if (mProfileDetailedView)
        return mProfileDetailedView->mAssociated ? ProfileAssociated(mProfileDetailedView->mAssociated) : ProfileAssociated{};

    return {};
}

ProfileViewerState BasicProfile::getViewer() const
{
    if (mViewer)
        return *mViewer;

    if (mProfileBasicView)
        return mProfileBasicView->mViewer ? ProfileViewerState(*mProfileBasicView->mViewer) : ProfileViewerState{};

    if (mProfileView)
        return mProfileView->mViewer ? ProfileViewerState(*mProfileView->mViewer) : ProfileViewerState{};

    if (mProfileDetailedView)
        return mProfileDetailedView->mViewer ? ProfileViewerState(*mProfileDetailedView->mViewer) : ProfileViewerState{};

    return {};
}

ContentLabelList BasicProfile::getContentLabels() const
{
    if (mContentLabels)
        return *mContentLabels;

    const ContentFilter::LabelList* labels = nullptr;

    if (mProfileBasicView)
        labels = &mProfileBasicView->mLabels;
    else if (mProfileView)
        labels = &mProfileView->mLabels;
    else if (mProfileDetailedView)
        labels = &mProfileDetailedView->mLabels;

    if (!labels)
        return {};

    return ContentFilter::getContentLabels(*labels);
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

bool BasicProfile::isFixedLabeler() const
{
    return ContentFilter::isFixedLabelerSubscription(getDid());
}

bool BasicProfile::canSendDirectMessage() const
{
    const auto allowIncoming = getAssociated().getChat().getAllowIncoming();

    switch (allowIncoming)
    {
    case QEnums::ALLOW_INCOMING_CHAT_NONE:
        return false;
    case QEnums::ALLOW_INCOMING_CHAT_ALL:
        return true;
    case QEnums::ALLOW_INCOMING_CHAT_FOLLOWING:
        return !getViewer().getFollowedBy().isEmpty();
    }

    qWarning() << "Unknown allow incoming value:" << allowIncoming;
    return false;
}

bool BasicProfile::isBlocked() const
{
    const auto viewer = getViewer();
    return viewer.isBlockedBy() || !viewer.getBlocking().isEmpty() || !viewer.getBlockingByList().isNull();
}

Profile::Profile(const ATProto::AppBskyActor::ProfileView::SharedPtr& profile) :
    BasicProfile(profile)
{
}

Profile::Profile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile) :
    BasicProfile(profile)
{
}

QString Profile::getDescription() const
{
    if (mDescription)
        return *mDescription;

    if (mProfileView)
        return mProfileView->mDescription.value_or("");

    if (mProfileDetailedView)
        return mProfileDetailedView->mDescription.value_or("");

    return {};
}

DetailedProfile::DetailedProfile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile) :
    Profile(profile)
{
}

QString DetailedProfile::getBanner() const
{
    return mProfileDetailedView ? mProfileDetailedView->mBanner.value_or("") : "";
}

int DetailedProfile::getFollowersCount() const
{
    return mProfileDetailedView ? mProfileDetailedView->mFollowersCount : 0;
}

int DetailedProfile::getFollowsCount() const
{
    return mProfileDetailedView ? mProfileDetailedView->mFollowsCount : 0;
}

int DetailedProfile::getPostsCount() const
{
    return mProfileDetailedView ? mProfileDetailedView->mPostsCount : 0;
}

}
