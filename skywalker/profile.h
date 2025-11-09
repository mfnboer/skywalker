// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "external_view.h"
#include "image_view.h"
#include "list_view_include.h"
#include "shared_image_provider.h"
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class BasicProfile;

class VerificationView
{
    Q_GADGET
    Q_PROPERTY(QString issuer READ getIssuer FINAL)
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(bool isValid READ isValid FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    QML_VALUE_TYPE(verificationview)

public:
    using List = QList<VerificationView>;

    VerificationView() = default;
    explicit VerificationView(const ATProto::AppBskyActor::VerificationView::SharedPtr& view);

    Q_INVOKABLE bool isNull() const { return mView == nullptr; }
    const QString& getIssuer() const; // DID
    const QString& getUri() const; // Verification record at-uri
    bool isValid() const;
    QDateTime getCreatedAt() const;

private:
    ATProto::AppBskyActor::VerificationView::SharedPtr mView;
};

class VerificationState
{
    Q_GADGET
    Q_PROPERTY(VerificationView::List verifications READ getVerifications FINAL)
    Q_PROPERTY(QEnums::VerifiedStatus verifiedStatus READ getVerifiedStatus FINAL)
    Q_PROPERTY(QEnums::VerifiedStatus trustedVerifierStatus READ getTrustedVerifierStatus FINAL)
    QML_VALUE_TYPE(verificationstate)

public:
    VerificationState() = default;
    explicit VerificationState(const ATProto::AppBskyActor::VerificationState& verificationState);

    Q_INVOKABLE bool isNull() const { return !mSet; }
    const VerificationView::List& getVerifications() const { return mVerifications; }
    QEnums::VerifiedStatus getVerifiedStatus() const { return mVerifiedStatus; }
    QEnums::VerifiedStatus getTrustedVerifierStatus() const { return mTrustedVerifierStatus; }
    Q_INVOKABLE VerificationView::List getValidVerifications() const;

private:
    bool mSet = false;
    VerificationView::List mVerifications;
    QEnums::VerifiedStatus mVerifiedStatus = QEnums::VERIFIED_STATUS_NONE;
    QEnums::VerifiedStatus mTrustedVerifierStatus = QEnums::VERIFIED_STATUS_NONE;
};

class ActorStatusView
{
    Q_GADGET
    Q_PROPERTY(QEnums::ActorStatus actorStatus READ getActorStatus FINAL)
    Q_PROPERTY(ExternalView externalView READ getExternalView FINAL)
    Q_PROPERTY(QDateTime expiresAt READ getExpiresAt FINAL)
    Q_PROPERTY(bool isActive READ isActive FINAL)
    QML_VALUE_TYPE(actorstatusview)

public:
    ActorStatusView() = default;
    explicit ActorStatusView(const ATProto::AppBskyActor::StatusView& statusView);

    Q_INVOKABLE bool isNull() const { return !mSet; }
    QEnums::ActorStatus getActorStatus() const { return mActorStatus; }
    const ExternalView& getExternalView() const { return mExternalView; }
    QDateTime getExpiresAt() const { return mExpiresAt; }
    bool isActive() const;

private:
    bool mSet = false;
    QEnums::ActorStatus mActorStatus;
    ExternalView mExternalView; // may be null
    QDateTime mExpiresAt; // may be null
    bool mIsActive = false;
};

class KnownFollowers
{
    Q_GADGET
    Q_PROPERTY(int count READ getCount FINAL)
    Q_PROPERTY(QList<BasicProfile> followers READ getFollowers FINAL)
    QML_VALUE_TYPE(knownfollowers)

public:
    KnownFollowers() = default;
    explicit KnownFollowers(const ATProto::AppBskyActor::KnownFollowers* knownFollowers);

    int getCount() const { return mPrivate ? mPrivate->mCount : 0; }
    const QList<BasicProfile>& getFollowers() const;

private:
    struct PrivateData
    {
        int mCount = 0;
        QList<BasicProfile> mFollowers;
    };
    std::shared_ptr<PrivateData> mPrivate;
};

class ActivitySubscription
{
    Q_GADGET
    Q_PROPERTY(bool post READ getPost FINAL)
    Q_PROPERTY(bool reply READ getReply FINAL)
    Q_PROPERTY(bool isSubscribed READ isSubscribed FINAL)
    QML_VALUE_TYPE(activitysubscription)

public:
    ActivitySubscription() = default;
    explicit ActivitySubscription(const ATProto::AppBskyNotification::ActivitySubscription* activitySubscription);

    bool getPost() const { return mPost; }
    bool getReply() const { return mReply; }
    bool isSubscribed() const { return mPost || mReply; }

private:
    bool mPost = false;
    bool mReply = false;
};

class ProfileViewerState
{
    Q_GADGET
    Q_PROPERTY(bool valid READ isValid FINAL)
    Q_PROPERTY(bool muted READ isMuted FINAL)
    Q_PROPERTY(bool blockedBy READ isBlockedBy FINAL)
    Q_PROPERTY(QString blocking READ getBlocking FINAL)
    Q_PROPERTY(QString following READ getFollowing FINAL)
    Q_PROPERTY(QString followedBy READ getFollowedBy FINAL)
    Q_PROPERTY(ListViewBasic mutedByList READ getMutedByList FINAL)
    Q_PROPERTY(ListViewBasic blockingByList READ getBlockingByList FINAL)
    Q_PROPERTY(KnownFollowers knownFollowers READ getKnownFollowers FINAL)
    Q_PROPERTY(ActivitySubscription activitySubscription READ getActivitySubscription FINAL)
    QML_VALUE_TYPE(profileviewerstate)

public:
    ProfileViewerState() = default;
    explicit ProfileViewerState(const ATProto::AppBskyActor::ViewerState::SharedPtr& viewerState);

    bool isValid() const;
    bool isMuted() const;
    bool isBlockedBy() const;
    const QString& getBlocking() const;
    const QString& getFollowing() const;
    const QString& getFollowedBy() const;
    const ListViewBasic& getMutedByList() const;
    const ListViewBasic& getBlockingByList() const;
    const KnownFollowers& getKnownFollowers() const;
    const ActivitySubscription& getActivitySubscription() const;

    void setBlocking(const QString& blocking);

private:
    struct PrivateData
    {
        ATProto::AppBskyActor::ViewerState::SharedPtr mViewerState;
        std::optional<ListViewBasic> mMutedByList;
        std::optional<ListViewBasic> mBlockedByList;
        std::optional<KnownFollowers> mKnownFollowers;
        std::optional<ActivitySubscription> mActivitySubscription;
    };
    std::shared_ptr<PrivateData> mPrivate;
};

class ProfileAssociatedChat
{
    Q_GADGET
    Q_PROPERTY(QEnums::AllowIncomingChat allowIncoming READ getAllowIncoming FINAL)
    QML_VALUE_TYPE(profileassociatedchat)

public:
    ProfileAssociatedChat() = default;
    explicit ProfileAssociatedChat(const ATProto::AppBskyActor::ProfileAssociatedChat::SharedPtr& associated);

    QEnums::AllowIncomingChat getAllowIncoming() const;

private:
    ATProto::AppBskyActor::ProfileAssociatedChat::SharedPtr mAssociated;
};

class ProfileAssociatedActivitySubscription
{
    Q_GADGET
    Q_PROPERTY(QEnums::AllowActivitySubscriptionsType allowSubscriptions READ getAllowSubscriptions FINAL)
    QML_VALUE_TYPE(profileassociatedactivitysubscription)
public:
    ProfileAssociatedActivitySubscription() = default;
    explicit ProfileAssociatedActivitySubscription(const ATProto::AppBskyActor::ProfileAssociatedActivitySubscription::SharedPtr& associated);

    QEnums::AllowActivitySubscriptionsType getAllowSubscriptions() const;

private:
    ATProto::AppBskyActor::ProfileAssociatedActivitySubscription::SharedPtr mAssociated;
};

class ProfileAssociated
{
    Q_GADGET
    Q_PROPERTY(int lists READ getLists FINAL)
    Q_PROPERTY(int feeds READ getFeeds FINAL)
    Q_PROPERTY(int starterPacks READ getStarterPacks FINAL)
    Q_PROPERTY(int isLabeler READ isLabeler FINAL)
    Q_PROPERTY(ProfileAssociatedChat chat READ getChat FINAL)
    Q_PROPERTY(ProfileAssociatedActivitySubscription activitySubscription READ getActivitySubscription FINAL)
    QML_VALUE_TYPE(profileassociated)

public:
    ProfileAssociated() = default;
    explicit ProfileAssociated(const ATProto::AppBskyActor::ProfileAssociated::SharedPtr& associated);

    int getLists() const { return mAssociated ? mAssociated->mLists : 0; }
    int getFeeds() const { return mAssociated ? mAssociated->mFeeds : 0; }
    int getStarterPacks() const { return mAssociated ? mAssociated->mStarterPacks : 0; }
    bool isLabeler() const { return mAssociated ? mAssociated->mLabeler : false; }
    ProfileAssociatedChat getChat() const { return mAssociated ? ProfileAssociatedChat(mAssociated->mChat) : ProfileAssociatedChat{}; }
    ProfileAssociatedActivitySubscription getActivitySubscription() const { return mAssociated ? ProfileAssociatedActivitySubscription(mAssociated->mActivitySubscription) : ProfileAssociatedActivitySubscription{}; }

private:
    ATProto::AppBskyActor::ProfileAssociated::SharedPtr mAssociated;
};

class BasicProfile
{
    Q_GADGET
    Q_PROPERTY(QString did READ getDid FINAL)
    Q_PROPERTY(QString handle READ getHandle FINAL)
    Q_PROPERTY(QString handleOrDid READ getHandleOrDid FINAL)
    Q_PROPERTY(QString displayName READ getDisplayName FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(QString pronouns READ getPronouns FINAL)
    Q_PROPERTY(QString avatarUrl READ getAvatarUrl FINAL)
    Q_PROPERTY(QString avatarThumbUrl READ getAvatarThumbUrl FINAL)
    Q_PROPERTY(ImageView imageView READ getImageView FINAL)
    Q_PROPERTY(ProfileAssociated associated READ getAssociated FINAL)
    Q_PROPERTY(ProfileViewerState viewer READ getViewer FINAL)
    Q_PROPERTY(ContentLabelList labels READ getContentLabels FINAL)
    Q_PROPERTY(bool hasCreatedAt READ hasCreatedAt FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(VerificationState verificationState READ getVerificationState FINAL)
    Q_PROPERTY(ActorStatusView actorStatus READ getActorStatus FINAL)
    QML_VALUE_TYPE(basicprofile)

public:
    BasicProfile() = default;
    BasicProfile(const BasicProfile&) = default;
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic::SharedPtr& profile);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileView::SharedPtr& profile);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile);
    BasicProfile(const QString& did, const QString& handle, const QString& displayName,
                 const QString& avatarUrl, const ProfileAssociated& associated = {},
                 const ProfileViewerState& viewer = {},
                 const ContentLabelList& contentLabels = {});

    BasicProfile& operator=(const BasicProfile&) = default;

    Q_INVOKABLE bool isNull() const;
    const QString& getDid() const;
    QString getName() const;
    const QString& getDisplayName() const;
    const QString& getHandle() const;
    const QString& getPronouns() const;
    const QString& getAvatarUrl() const;
    QString getAvatarThumbUrl() const;
    ImageView getImageView() const;
    ProfileAssociated getAssociated() const;
    ProfileViewerState& getViewer();
    const ProfileViewerState& getViewer() const;
    const ContentLabelList& getContentLabels() const;
    bool hasCreatedAt() const;
    QDateTime getCreatedAt() const; // can return null datetime
    VerificationState& getVerificationState();
    const VerificationState& getVerificationState() const;
    ActorStatusView& getActorStatus();
    const ActorStatusView& getActorStatus() const;

    Q_INVOKABLE bool hasInvalidHandle() const;

    // Get the handle, but if it is invalid then get the DID
    const QString& getHandleOrDid() const;

    void setDisplayName(const QString& displayName);
    void setPronouns(const QString& pronouns);

    // If avatarUrl is a "image://", then the profile takes ownership of the image
    void setAvatarUrl(const QString& avatarUrl);

    Q_INVOKABLE bool isFixedLabeler() const;
    Q_INVOKABLE bool canSendDirectMessage() const;
    Q_INVOKABLE bool allowsActivitySubscriptions() const;

    // Check if communication is blocked either due to the user blocking it
    // or being blocked by it.
    Q_INVOKABLE bool isBlocked() const;

    ATProto::AppBskyActor::ProfileViewBasic::SharedPtr getProfileBasicView() const;

    bool operator==(const BasicProfile& other) const
    {
        return getDid() == other.getDid();
    }

    struct Hash
    {
        size_t operator()(const BasicProfile& profile) const
        {
            return qHash(profile.getDid());
        }
    };

protected:
    ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr mProfileDetailedView;
    ATProto::AppBskyActor::ProfileView::SharedPtr mProfileView;

private:
    struct PrivateData
    {
        ATProto::AppBskyActor::ProfileViewBasic::SharedPtr mProfileBasicView;
        std::optional<QString> mDid;
        std::optional<QString> mHandle;
        std::optional<QString> mDisplayName;
        std::optional<QString> mPronouns;
        std::optional<QString> mAvatarUrl;
        SharedImageSource::SharedPtr mAvatarSource;
        std::optional<ProfileAssociated> mAssociated;
        std::optional<ProfileViewerState> mViewer;
        std::optional<ContentLabelList> mContentLabels;
        std::optional<QDateTime> mCreatedAt;
        std::optional<VerificationState> mVerificationState;
        std::optional<ActorStatusView> mActorStatus;
    };
    std::shared_ptr<PrivateData> mPrivate;
};

using BasicProfileList = QList<BasicProfile>;

class Profile : public BasicProfile
{
    Q_GADGET
    Q_PROPERTY(QString description READ getDescription FINAL)
    QML_VALUE_TYPE(profile)

public:
    Profile() = default;
    explicit Profile(const ATProto::AppBskyActor::ProfileView::SharedPtr& profile);
    explicit Profile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile);
    Profile(const QString& did, const QString& handle, const QString& displayName,
            const QString& avatarUrl);

    QString getDescription() const;
    void setDescription(const QString& description) { mDescription = description; }

private:
    std::optional<QString> mDescription;
};

using ProfileList = QList<Profile>;

class DetailedProfile : public Profile
{
    Q_GADGET
    Q_PROPERTY(QString website READ getWebsite FINAL)
    Q_PROPERTY(QString banner READ getBanner FINAL)
    Q_PROPERTY(int followersCount READ getFollowersCount FINAL)
    Q_PROPERTY(int followsCount READ getFollowsCount FINAL)
    Q_PROPERTY(int postsCount READ getPostsCount FINAL)
    Q_PROPERTY(QString pinnedPostUri READ getPinnedPostUri FINAL)
    QML_VALUE_TYPE(detailedprofile)

public:
    DetailedProfile() = default;
    explicit DetailedProfile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile);

    QString getWebsite() const;
    QString getBanner() const;
    int getFollowersCount() const;
    int getFollowsCount() const;
    int getPostsCount() const;
    QString getPinnedPostUri() const;
};

}

Q_DECLARE_METATYPE(::Skywalker::ProfileViewerState)
Q_DECLARE_METATYPE(::Skywalker::ProfileAssociated)
Q_DECLARE_METATYPE(::Skywalker::BasicProfile)
Q_DECLARE_METATYPE(::Skywalker::Profile)
Q_DECLARE_METATYPE(::Skywalker::DetailedProfile)
