// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "image_view.h"
#include "list_view_include.h"
#include "shared_image_provider.h"
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class BasicProfile;

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

    void setBlocking(const QString& blocking);

private:
    struct PrivateData
    {
        ATProto::AppBskyActor::ViewerState::SharedPtr mViewerState;
        std::optional<ListViewBasic> mMutedByList;
        std::optional<ListViewBasic> mBlockedByList;
        std::optional<KnownFollowers> mKnownFollowers;
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

class ProfileAssociated
{
    Q_GADGET
    Q_PROPERTY(int lists READ getLists FINAL)
    Q_PROPERTY(int feeds READ getFeeds FINAL)
    Q_PROPERTY(int starterPacks READ getStarterPacks FINAL)
    Q_PROPERTY(int isLabeler READ isLabeler FINAL)
    Q_PROPERTY(ProfileAssociatedChat chat READ getChat FINAL)
    QML_VALUE_TYPE(profileassociated)

public:
    ProfileAssociated() = default;
    explicit ProfileAssociated(const ATProto::AppBskyActor::ProfileAssociated::SharedPtr& associated);

    int getLists() const { return mAssociated ? mAssociated->mLists : 0; }
    int getFeeds() const { return mAssociated ? mAssociated->mFeeds : 0; }
    int getStarterPacks() const { return mAssociated ? mAssociated->mStarterPacks : 0; }
    bool isLabeler() const { return mAssociated ? mAssociated->mLabeler : false; }
    ProfileAssociatedChat getChat() const { return mAssociated ? ProfileAssociatedChat(mAssociated->mChat) : ProfileAssociatedChat{}; }

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
    Q_PROPERTY(QString avatarUrl READ getAvatarUrl FINAL)
    Q_PROPERTY(QString avatarThumbUrl READ getAvatarThumbUrl FINAL)
    Q_PROPERTY(ImageView imageView READ getImageView FINAL)
    Q_PROPERTY(ProfileAssociated associated READ getAssociated FINAL)
    Q_PROPERTY(ProfileViewerState viewer READ getViewer FINAL)
    Q_PROPERTY(ContentLabelList labels READ getContentLabels FINAL)
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
    const QString& getAvatarUrl() const;
    QString getAvatarThumbUrl() const;
    ImageView getImageView() const;
    ProfileAssociated getAssociated() const;
    ProfileViewerState& getViewer();
    const ProfileViewerState& getViewer() const;
    const ContentLabelList& getContentLabels() const;

    Q_INVOKABLE bool hasInvalidHandle() const;

    // Get the handle, but if it is invalid then get the DID
    const QString& getHandleOrDid() const;

    void setDisplayName(const QString& displayName);

    // If avatarUrl is a "image://", then the profile takes ownership of the image
    void setAvatarUrl(const QString& avatarUrl);

    Q_INVOKABLE bool isFixedLabeler() const;
    Q_INVOKABLE bool canSendDirectMessage() const;

    // Check if communication is blocked either due to the user blocking it
    // or being blocked by it.
    Q_INVOKABLE bool isBlocked() const;

    ATProto::AppBskyActor::ProfileViewBasic::SharedPtr getProfileBasicView() const;

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
        std::optional<QString> mAvatarUrl;
        SharedImageSource::SharedPtr mAvatarSource;
        std::optional<ProfileAssociated> mAssociated;
        std::optional<ProfileViewerState> mViewer;
        std::optional<ContentLabelList> mContentLabels;
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

    QString getDescription() const;
    void setDescription(const QString& description) { mDescription = description; }

private:
    std::optional<QString> mDescription;
};

using ProfileList = QList<Profile>;

class DetailedProfile : public Profile
{
    Q_GADGET
    Q_PROPERTY(QString banner READ getBanner FINAL)
    Q_PROPERTY(int followersCount READ getFollowersCount FINAL)
    Q_PROPERTY(int followsCount READ getFollowsCount FINAL)
    Q_PROPERTY(int postsCount READ getPostsCount FINAL)
    Q_PROPERTY(QString pinnedPostUri READ getPinnedPostUri FINAL)
    QML_VALUE_TYPE(detailedprofile)

public:
    DetailedProfile() = default;
    explicit DetailedProfile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile);

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
