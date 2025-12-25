// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile.h"
#include "author_cache.h"
#include "content_filter.h"
#include "definitions.h"
#include "list_cache.h"
#include "utils.h"

namespace Skywalker {

static const QString NULL_STRING;

VerificationView::VerificationView(const ATProto::AppBskyActor::VerificationView::SharedPtr& view) :
    mView(view)
{
}

const QString& VerificationView::getIssuer() const
{
    return mView ? mView->mIssuer : NULL_STRING;
}

const QString& VerificationView::getUri() const
{
    return mView ? mView->mUri : NULL_STRING;
}

bool VerificationView::isValid() const
{
    return mView ? mView->mIsValid : false;
}

QDateTime VerificationView::getCreatedAt() const
{
    return mView ? mView->mCreatedAt : QDateTime{};
}

VerificationState::VerificationState(const ATProto::AppBskyActor::VerificationState& verificationState) :
    mSet(true),
    mVerifiedStatus(QEnums::VerifiedStatus((int)verificationState.mVerifiedStatus)),
    mTrustedVerifierStatus(QEnums::VerifiedStatus((int)verificationState.mTrustedVerifierStatus))
{
    mVerifications.reserve(verificationState.mVerifications.size());

    for (const auto& verificationView : verificationState.mVerifications)
        mVerifications.push_back(VerificationView(verificationView));
}

VerificationView::List VerificationState::getValidVerifications() const
{
    VerificationView::List validVerifications;
    validVerifications.reserve(mVerifications.size());

    for (const auto& verification : mVerifications)
    {
        if (verification.isValid())
            validVerifications.push_back(verification);
    }

    return validVerifications;
}

ActorStatusView::ActorStatusView(const ATProto::AppBskyActor::StatusView& statusView) :
    mSet(true),
    mActorStatus(QEnums::ActorStatus((int)statusView.mStatus)),
    mExpiresAt(statusView.mExpiresAt.value_or(QDateTime{})),
    mIsActive(statusView.mIsActive.value_or(true))
{
    if (statusView.mEmbed && !ATProto::isNullVariant(*statusView.mEmbed) &&
        std::holds_alternative<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(*statusView.mEmbed))
    {
        auto view = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(*statusView.mEmbed);
        mExternalView = ExternalView(view->mExternal);
    }
}

bool ActorStatusView::isActive() const
{
    if (!mSet)
        return false;

    if (!mExpiresAt.isValid())
        return true;

    if (!mIsActive)
        return false;

    return mExpiresAt >= QDateTime::currentDateTime();
}

KnownFollowers::KnownFollowers(const ATProto::AppBskyActor::KnownFollowers* knownFollowers)
{
    if (!knownFollowers)
        return;

    mPrivate = std::make_shared<PrivateData>();

    mPrivate->mCount = knownFollowers->mCount;
    mPrivate->mFollowers.reserve(knownFollowers->mFollowers.size());

    for (const auto& follower : knownFollowers->mFollowers)
    {
        mPrivate->mFollowers.push_back(BasicProfile(follower));

        // Cap followers to the maximum to be safe in case the networks gives
        // much more.
        if (mPrivate->mFollowers.size() >= ATProto::AppBskyActor::KnownFollowers::MAX_COUNT)
            break;
    }
}

const QList<BasicProfile>& KnownFollowers::getFollowers() const
{
    if (!mPrivate)
    {
        static const QList<BasicProfile> NULL_LIST;
        return NULL_LIST;
    }

    return mPrivate->mFollowers;
}

ActivitySubscription::ActivitySubscription(const ATProto::AppBskyNotification::ActivitySubscription* activitySubscription)
{
    if (!activitySubscription)
        return;

    mPost = activitySubscription->mPost;
    mReply = activitySubscription->mReply;
}

ProfileViewerState::ProfileViewerState(const ATProto::AppBskyActor::ViewerState::SharedPtr& viewerState) :
    mPrivate{std::make_shared<PrivateData>(viewerState)}
{
}

bool ProfileViewerState::isValid() const
{
    return mPrivate != nullptr && mPrivate->mViewerState != nullptr;
}

bool ProfileViewerState::isMuted() const
{
    return mPrivate && mPrivate->mViewerState ? mPrivate->mViewerState->mMuted : false;
}

bool ProfileViewerState::isBlockedBy() const
{
    return mPrivate && mPrivate->mViewerState ? mPrivate->mViewerState->mBlockedBy : false;
}

const QString& ProfileViewerState::getBlocking() const
{
    return mPrivate && mPrivate->mViewerState && mPrivate->mViewerState->mBlocking ? *mPrivate->mViewerState->mBlocking : NULL_STRING;
}

void ProfileViewerState::setBlocking(const QString& blocking)
{
    if (mPrivate && mPrivate->mViewerState)
    {
        if (blocking.isEmpty())
            mPrivate->mViewerState->mBlocking = {};
        else
            mPrivate->mViewerState->mBlocking = blocking;
    }
    else
    {
        qWarning() << "No viewer state to set blocking:" << blocking;
    }
}

const QString& ProfileViewerState::getFollowing() const
{
    return mPrivate && mPrivate->mViewerState && mPrivate->mViewerState->mFollowing ? *mPrivate->mViewerState->mFollowing : NULL_STRING;
}

const QString& ProfileViewerState::getFollowedBy() const
{
    return mPrivate && mPrivate->mViewerState && mPrivate->mViewerState->mFollowedBy ? *mPrivate->mViewerState->mFollowedBy : NULL_STRING;
}

const ListViewBasic& ProfileViewerState::getMutedByList() const
{
    if (mPrivate && mPrivate->mMutedByList)
        return *mPrivate->mMutedByList;

    if (mPrivate)
    {
        if (mPrivate->mViewerState && mPrivate->mViewerState->mMutedByList)
            mPrivate->mMutedByList = ListViewBasic(mPrivate->mViewerState->mMutedByList);
        else
            mPrivate->mMutedByList = ListViewBasic{};
    }
    else
    {
        const_cast<ProfileViewerState*>(this)->mPrivate = std::make_shared<PrivateData>();
        mPrivate->mMutedByList = ListViewBasic{};
    }

    return *mPrivate->mMutedByList;
}

const ListViewBasic& ProfileViewerState::getBlockingByList() const
{
    if (mPrivate && mPrivate->mBlockedByList)
        return *mPrivate->mBlockedByList;

    if (mPrivate)
    {
        if (mPrivate->mViewerState && mPrivate->mViewerState->mBlockingByList)
           mPrivate->mBlockedByList = ListViewBasic(mPrivate->mViewerState->mBlockingByList);
        else
            mPrivate->mBlockedByList = ListViewBasic{};
    }
    else
    {
        const_cast<ProfileViewerState*>(this)->mPrivate = std::make_shared<PrivateData>();
        mPrivate->mBlockedByList = ListViewBasic{};
    }

    return *mPrivate->mBlockedByList;
}

const KnownFollowers& ProfileViewerState::getKnownFollowers() const
{
    if (mPrivate && mPrivate->mKnownFollowers)
        return *mPrivate->mKnownFollowers;

    if (mPrivate)
    {
        mPrivate->mKnownFollowers = mPrivate->mViewerState ? KnownFollowers(mPrivate->mViewerState->mKnownFollowers.get()) : KnownFollowers{};
    }
    else
    {
        const_cast<ProfileViewerState*>(this)->mPrivate = std::make_shared<PrivateData>();
        mPrivate->mKnownFollowers = KnownFollowers{};
    }

    return *mPrivate->mKnownFollowers;
}

const ActivitySubscription& ProfileViewerState::getActivitySubscription() const
{
    if (mPrivate && mPrivate->mActivitySubscription)
        return *mPrivate->mActivitySubscription;

    if (mPrivate)
    {
        mPrivate->mActivitySubscription = mPrivate->mViewerState ? ActivitySubscription(mPrivate->mViewerState->mActivitySubscription.get()) : ActivitySubscription{};
    }
    else
    {
        const_cast<ProfileViewerState*>(this)->mPrivate = std::make_shared<PrivateData>();
        mPrivate->mActivitySubscription = ActivitySubscription{};
    }

    return *mPrivate->mActivitySubscription;
}

ATProto::AppBskyActor::ViewerState::SharedPtr ProfileViewerState::getViewerState() const
{
    return mPrivate ? mPrivate->mViewerState : nullptr;
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


ProfileAssociatedActivitySubscription::ProfileAssociatedActivitySubscription(const ATProto::AppBskyActor::ProfileAssociatedActivitySubscription::SharedPtr& associated) :
    mAssociated(associated)
{
}

QEnums::AllowActivitySubscriptionsType ProfileAssociatedActivitySubscription::getAllowSubscriptions() const
{
    // Default bsky setting is followers
    return mAssociated ? (QEnums::AllowActivitySubscriptionsType)mAssociated->mAllowSubscriptions : QEnums::ALLOW_ACTIVITY_SUBSCRIPTIONS_FOLLOWERS;
}


ProfileAssociated::ProfileAssociated(const ATProto::AppBskyActor::ProfileAssociated::SharedPtr& associated) :
    mAssociated(associated)
{
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic::SharedPtr& profile) :
    mPrivate{std::make_shared<PrivateData>(profile)}
{
    Q_ASSERT(mPrivate->mProfileBasicView);
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
    mPrivate{std::make_shared<PrivateData>(
        nullptr,
        did,
        handle,
        displayName,
        std::nullopt,
        avatarUrl,
        nullptr,
        associated,
        viewer,
        contentLabels)}
{
}

bool BasicProfile::isNull() const
{
    return getDid().isEmpty();
}

const QString& BasicProfile::getDid() const
{
    if (mPrivate)
    {
        if (mPrivate->mDid)
            return *mPrivate->mDid;

        if (mPrivate->mProfileBasicView)
            return mPrivate->mProfileBasicView->mDid;
    }

    if (mProfileView)
        return mProfileView->mDid;

    if (mProfileDetailedView)
        return mProfileDetailedView->mDid;

    return NULL_STRING;
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

const QString& BasicProfile::getDisplayName() const
{
    if (mPrivate)
    {
        if (mPrivate->mDisplayName)
            return *mPrivate->mDisplayName;

        if (mPrivate->mProfileBasicView)
            return mPrivate->mProfileBasicView->mDisplayName ? *mPrivate->mProfileBasicView->mDisplayName : NULL_STRING;
    }

    if (mProfileView)
        return mProfileView->mDisplayName ? *mProfileView->mDisplayName : NULL_STRING;

    if (mProfileDetailedView)
        return mProfileDetailedView->mDisplayName ? *mProfileDetailedView->mDisplayName : NULL_STRING;

    return NULL_STRING;
}

const QString& BasicProfile::getPronouns() const
{
    if (mPrivate)
    {
        if (mPrivate->mPronouns)
            return *mPrivate->mPronouns;

        if (mPrivate->mProfileBasicView)
            return mPrivate->mProfileBasicView->mPronouns ? *mPrivate->mProfileBasicView->mPronouns : NULL_STRING;
    }

    if (mProfileView)
        return mProfileView->mPronouns ? *mProfileView->mPronouns : NULL_STRING;

    if (mProfileDetailedView)
        return mProfileDetailedView->mPronouns ? *mProfileDetailedView->mPronouns : NULL_STRING;

    return NULL_STRING;
}

const QString& BasicProfile::getHandle() const
{
    if (mPrivate)
    {
        if (mPrivate->mHandle)
            return *mPrivate->mHandle;

        if (mPrivate->mProfileBasicView)
            return mPrivate->mProfileBasicView->mHandle;
    }

    if (mProfileView)
        return mProfileView->mHandle;

    if (mProfileDetailedView)
        return mProfileDetailedView->mHandle;

    return NULL_STRING;
}

bool BasicProfile::hasInvalidHandle() const
{
    return getHandle().endsWith(INVALID_HANDLE_SUFFIX);
}

const QString& BasicProfile::getHandleOrDid() const
{
    const QString& handle = getHandle();

    if (handle.isEmpty() || handle.endsWith(INVALID_HANDLE_SUFFIX))
        return getDid();

    return handle;
}

const QString& BasicProfile::getAvatarUrl() const
{
    if (mPrivate)
    {
        if (mPrivate->mAvatarUrl)
            return *mPrivate->mAvatarUrl;

        if (mPrivate->mProfileBasicView)
            return mPrivate->mProfileBasicView->mAvatar ? *mPrivate->mProfileBasicView->mAvatar : NULL_STRING;
    }

    if (mProfileView)
        return mProfileView->mAvatar? *mProfileView->mAvatar : NULL_STRING;

    if (mProfileDetailedView)
        return mProfileDetailedView->mAvatar ? *mProfileDetailedView->mAvatar : NULL_STRING;

    return NULL_STRING;
}

QString BasicProfile::getAvatarThumbUrl() const
{
    return ATProto::createAvatarThumbUrl(getAvatarUrl());
}

ImageView BasicProfile::getImageView() const
{
    return ImageView(getAvatarUrl(), getName());
}

ProfileAssociated BasicProfile::getAssociated() const
{
    if (mPrivate)
    {
        if (mPrivate->mAssociated)
            return *mPrivate->mAssociated;

        if (mPrivate->mProfileBasicView)
            return mPrivate->mProfileBasicView->mAssociated ? ProfileAssociated(mPrivate->mProfileBasicView->mAssociated) : ProfileAssociated{};
    }

    if (mProfileView)
        return mProfileView->mAssociated ? ProfileAssociated(mProfileView->mAssociated) : ProfileAssociated{};

    if (mProfileDetailedView)
        return mProfileDetailedView->mAssociated ? ProfileAssociated(mProfileDetailedView->mAssociated) : ProfileAssociated{};

    return {};
}

void BasicProfile::setViewer(const QString& followingUri)
{
    if (!mPrivate)
        mPrivate = std::make_shared<PrivateData>();

    auto viewerState = std::make_shared<ATProto::AppBskyActor::ViewerState>();
    viewerState->mFollowing = followingUri;
    mPrivate->mViewer = ProfileViewerState{viewerState};
}

ProfileViewerState& BasicProfile::getViewer()
{
    if (mPrivate)
    {
        if (mPrivate->mViewer)
            return *mPrivate->mViewer;

        if (mPrivate->mProfileBasicView)
            mPrivate->mViewer = mPrivate->mProfileBasicView->mViewer ? ProfileViewerState(mPrivate->mProfileBasicView->mViewer) : ProfileViewerState{};
        else
            mPrivate->mViewer = ProfileViewerState{};

        return *mPrivate->mViewer;
    }
    else
    {
        mPrivate = std::make_shared<PrivateData>();
    }

    if (mProfileView)
        mPrivate->mViewer = mProfileView->mViewer ? ProfileViewerState(mProfileView->mViewer) : ProfileViewerState{};
    else if (mProfileDetailedView)
        mPrivate->mViewer = mProfileDetailedView->mViewer ? ProfileViewerState(mProfileDetailedView->mViewer) : ProfileViewerState{};
    else
        mPrivate->mViewer = ProfileViewerState{};

    return *mPrivate->mViewer;
}

const ProfileViewerState& BasicProfile::getViewer() const
{
    ProfileViewerState& viewer = const_cast<BasicProfile*>(this)->getViewer();
    return viewer;
}

const ContentLabelList& BasicProfile::getContentLabels() const
{
    if (mPrivate && mPrivate->mContentLabels)
        return *mPrivate->mContentLabels;

    const ContentFilter::LabelList* labels = nullptr;

    if (mPrivate && mPrivate->mProfileBasicView)
        labels = &mPrivate->mProfileBasicView->mLabels;
    else if (mProfileView)
        labels = &mProfileView->mLabels;
    else if (mProfileDetailedView)
        labels = &mProfileDetailedView->mLabels;

    if (!mPrivate)
        const_cast<BasicProfile*>(this)->mPrivate = std::make_shared<PrivateData>();

    if (labels)
        mPrivate->mContentLabels = ContentFilter::getContentLabels(*labels);
    else
        mPrivate->mContentLabels = ContentLabelList{};

    return *mPrivate->mContentLabels;
}

bool BasicProfile::hasCreatedAt() const
{
    return getCreatedAt().isValid();
}

QDateTime BasicProfile::getCreatedAt() const
{
    if (mPrivate)
    {
        if (mPrivate->mCreatedAt)
            return *mPrivate->mCreatedAt;

        if (mPrivate->mProfileBasicView)
            return mPrivate->mProfileBasicView->mCreatedAt.value_or(QDateTime{});
    }

    if (mProfileView)
        return mProfileView->mCreatedAt.value_or(QDateTime{});

    if (mProfileDetailedView)
        return mProfileDetailedView->mCreatedAt.value_or(QDateTime{});

    return QDateTime{};
}

VerificationState& BasicProfile::getVerificationState()
{
    if (mPrivate)
    {
        if (mPrivate->mVerificationState)
            return *mPrivate->mVerificationState;

        if (mPrivate->mProfileBasicView)
            mPrivate->mVerificationState = mPrivate->mProfileBasicView->mVerification ? VerificationState(*mPrivate->mProfileBasicView->mVerification) : VerificationState{};
        else
            mPrivate->mVerificationState = VerificationState{};

        return *mPrivate->mVerificationState;
    }
    else
    {
        mPrivate = std::make_shared<PrivateData>();
    }

    if (mProfileView)
        mPrivate->mVerificationState = mProfileView->mVerification ? VerificationState(*mProfileView->mVerification) : VerificationState{};
    else if (mProfileDetailedView)
        mPrivate->mVerificationState = mProfileDetailedView->mVerification ? VerificationState(*mProfileDetailedView->mVerification) : VerificationState{};
    else
        mPrivate->mVerificationState = VerificationState{};

    return *mPrivate->mVerificationState;
}

const VerificationState& BasicProfile::getVerificationState() const
{
    VerificationState& state = const_cast<BasicProfile*>(this)->getVerificationState();
    return state;
}

ActorStatusView& BasicProfile::getActorStatus()
{
    if (mPrivate)
    {
        if (mPrivate->mActorStatus)
            return *mPrivate->mActorStatus;

        if (mPrivate->mProfileBasicView)
            mPrivate->mActorStatus = mPrivate->mProfileBasicView->mStatus ? ActorStatusView(*mPrivate->mProfileBasicView->mStatus) : ActorStatusView{};
        else
            mPrivate->mActorStatus = ActorStatusView{};

        return *mPrivate->mActorStatus;
    }
    else
    {
        mPrivate = std::make_shared<PrivateData>();
    }

    if (mProfileView)
        mPrivate->mActorStatus = mProfileView->mStatus ? ActorStatusView(*mProfileView->mStatus) : ActorStatusView{};
    else if (mProfileDetailedView)
        mPrivate->mActorStatus = mProfileDetailedView->mStatus ? ActorStatusView(*mProfileDetailedView->mStatus) : ActorStatusView{};
    else
        mPrivate->mActorStatus = ActorStatusView{};

    return *mPrivate->mActorStatus;
}

const ActorStatusView& BasicProfile::getActorStatus() const
{
    ActorStatusView& status = const_cast<BasicProfile*>(this)->getActorStatus();
    return status;
}

void BasicProfile::setDisplayName(const QString& displayName)
{
    if (!mPrivate)
        mPrivate = std::make_shared<PrivateData>();

    mPrivate->mDisplayName = displayName;
}

void BasicProfile::setPronouns(const QString& pronouns)
{
    if (!mPrivate)
        mPrivate = std::make_shared<PrivateData>();

    mPrivate->mPronouns = pronouns;
}

void BasicProfile::setAvatarUrl(const QString& avatarUrl)
{
    if (!mPrivate)
        mPrivate = std::make_shared<PrivateData>();

    mPrivate->mAvatarUrl = avatarUrl;

    if (avatarUrl.startsWith("image://"))
    {
        auto* provider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
        mPrivate->mAvatarSource = std::make_shared<SharedImageSource>(avatarUrl, provider);
    }
    else
    {
        mPrivate->mAvatarSource = nullptr;
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

bool BasicProfile::allowsActivitySubscriptions() const
{
    const auto allowSubscriptions = getAssociated().getActivitySubscription().getAllowSubscriptions();

    switch (allowSubscriptions)
    {
    case QEnums::ALLOW_ACTIVITY_SUBSCRIPTIONS_NONE:
        return false;
    case QEnums::ALLOW_ACTIVITY_SUBSCRIPTIONS_FOLLOWERS:
        return !getViewer().getFollowing().isEmpty();
    case QEnums::ALLOW_ACTIVITY_SUBSCRIPTIONS_MUTUALS:
        return !getViewer().getFollowing().isEmpty() && !getViewer().getFollowedBy().isEmpty();
    }

    return false;
}

bool BasicProfile::isBlocked() const
{
    const auto viewer = getViewer();
    return viewer.isBlockedBy() || !viewer.getBlocking().isEmpty() || !viewer.getBlockingByList().isNull();
}

template<class ProfileType>
static ATProto::AppBskyActor::ProfileViewBasic::SharedPtr createProfileBasicView(ProfileType p)
{
    auto profile = std::make_shared<ATProto::AppBskyActor::ProfileViewBasic>();
    profile->mDid = p->mDid;
    profile->mHandle = p->mHandle;
    profile->mDisplayName = p->mDisplayName;
    profile->mPronouns = p->mPronouns;
    profile->mAvatar = p->mAvatar;
    profile->mAssociated = p->mAssociated;
    profile->mViewer = p->mViewer;
    profile->mLabels = p->mLabels;
    profile->mCreatedAt = p->mCreatedAt;
    profile->mVerification = p->mVerification;
    profile->mStatus = p->mStatus;
    return profile;
}

ATProto::AppBskyActor::ProfileViewBasic::SharedPtr BasicProfile::getProfileBasicView() const
{
    if (!mPrivate)
    {
        if (mProfileDetailedView)
            return createProfileBasicView(mProfileDetailedView);

        if (mProfileView)
            return createProfileBasicView(mProfileView);

        return nullptr;
    }

    if (mPrivate->mProfileBasicView)
        return mPrivate->mProfileBasicView;

    auto profile = std::make_shared<ATProto::AppBskyActor::ProfileViewBasic>();
    profile->mDid = getDid();
    profile->mHandle = getHandle();
    profile->mDisplayName = Utils::makeOptionalString(getDisplayName());
    profile->mPronouns = Utils::makeOptionalString(getPronouns());
    profile->mAvatar = Utils::makeOptionalString(getAvatarUrl());
    profile->mAssociated = getAssociated().getAssociated();
    profile->mViewer = getViewer().getViewerState();

    if (hasCreatedAt())
        profile->mCreatedAt = getCreatedAt();

    return profile;
}

Profile::Profile(const ATProto::AppBskyActor::ProfileView::SharedPtr& profile) :
    BasicProfile(profile)
{
}

Profile::Profile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile) :
    BasicProfile(profile)
{
}

Profile::Profile(const QString& did, const QString& handle, const QString& displayName,
        const QString& avatarUrl) :
    BasicProfile(did, handle, displayName, avatarUrl)
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

QString DetailedProfile::getWebsite() const
{
    return mProfileDetailedView ? mProfileDetailedView->mWebsite.value_or("") : "";
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

QString DetailedProfile::getPinnedPostUri() const
{
    return mProfileDetailedView && mProfileDetailedView->mPinnedPost ? mProfileDetailedView->mPinnedPost->mUri : "";
}


BlockedAuthor::BlockedAuthor(const ATProto::AppBskyFeed::BlockedAuthor::SharedPtr& blockedAuthor) :
    mBlockedAuthor(blockedAuthor)
{
}

bool BlockedAuthor::isNull() const
{
    return !mBlockedAuthor;
}

QString BlockedAuthor::getDid() const
{
    return mBlockedAuthor ? mBlockedAuthor->mDid : "";
}

BasicProfile BlockedAuthor::getAuthor() const
{
    const auto did = getDid();

    if (did.isEmpty())
        return {};

    auto* profile = AuthorCache::instance().get(did);
    return profile ? *profile : BasicProfile();
}

ProfileViewerState BlockedAuthor::getViewer() const
{
    if (!mBlockedAuthor || !mBlockedAuthor->mViewer)
        return {};

    return ProfileViewerState(mBlockedAuthor->mViewer);
}

QString BlockedAuthor::getBlockingByListUri() const
{
    if (!mBlockedAuthor || !mBlockedAuthor->mViewer)
        return {};

    if (mBlockedAuthor->mViewer->mBlockingByList)
        return mBlockedAuthor->mViewer->mBlockingByList->mUri;

    // It seems that blockingByList may not be filled in. Instead the blocking URI
    // is set to a list-URI
    if (!mBlockedAuthor->mViewer->mBlocking)
        return {};

    const ATProto::ATUri uri(*mBlockedAuthor->mViewer->mBlocking);

    if (uri.getCollection() != ATProto::ATUri::COLLECTION_GRAPH_LIST)
        return {};

    return *mBlockedAuthor->mViewer->mBlocking;
}

ListViewBasic BlockedAuthor::getBlockingByList() const
{
    if (!mBlockedAuthor || !mBlockedAuthor->mViewer)
        return {};

    if (mBlockedAuthor->mViewer->mBlockingByList)
        return ListViewBasic(mBlockedAuthor->mViewer->mBlockingByList);

    // It seems that blockingByList may not be filled in. Instead the blocking URI
    // is set to a list-URI
    if (!mBlockedAuthor->mViewer->mBlocking)
        return {};

    const ATProto::ATUri uri(*mBlockedAuthor->mViewer->mBlocking);

    if (uri.getCollection() != ATProto::ATUri::COLLECTION_GRAPH_LIST)
        return {};

    auto* list = ListCache::instance().get(*mBlockedAuthor->mViewer->mBlocking);
    return list ? *list : ListViewBasic();
}

}
