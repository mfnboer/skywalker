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

    int getCount() const { return mCount; }
    QList<BasicProfile> getFollowers() const;

private:
    int mCount = 0;
    QList<std::shared_ptr<BasicProfile>> mFollowers;
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
    explicit ProfileViewerState(const ATProto::AppBskyActor::ViewerState& viewerState);

    bool isValid() const { return mValid; }
    bool isMuted() const { return mMuted; }
    bool isBlockedBy() const { return mBlockedBy; }
    const QString& getBlocking() const { return mBlocking; }
    const QString& getFollowing() const { return mFollowing; }
    const QString& getFollowedBy() const { return mFollowedBy; }
    const ListViewBasic& getMutedByList() const { return mMutedByList; }
    const ListViewBasic& getBlockingByList() const { return mBlockingByList; }
    const KnownFollowers& getKnownFollowers() const { return mKnownFollowers; }

private:
    bool mValid = false;
    bool mMuted = false;
    bool mBlockedBy = false;
    QString mBlocking;
    QString mFollowing;
    QString mFollowedBy;
    ListViewBasic mMutedByList;
    ListViewBasic mBlockingByList;
    KnownFollowers mKnownFollowers;
};

class ProfileAssociatedChat
{
    Q_GADGET
    Q_PROPERTY(QEnums::AllowIncomingChat allowIncoming READ getAllowIncoming FINAL)
    QML_VALUE_TYPE(profileassociatedchat)

public:
    ProfileAssociatedChat() = default;
    explicit ProfileAssociatedChat(const ATProto::AppBskyActor::ProfileAssociatedChat& associated);

    QEnums::AllowIncomingChat getAllowIncoming() const { return mAllowIncoming; }

private:
    // Default bsky setting is following
    QEnums::AllowIncomingChat mAllowIncoming = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
};

class ProfileAssociated
{
    Q_GADGET
    Q_PROPERTY(int lists READ getLists FINAL)
    Q_PROPERTY(int feeds READ getFeeds FINAL)
    Q_PROPERTY(int isLabeler READ isLabeler FINAL)
    Q_PROPERTY(ProfileAssociatedChat chat READ getChat FINAL)
    QML_VALUE_TYPE(profileassociated)

public:
    ProfileAssociated() = default;
    explicit ProfileAssociated(const ATProto::AppBskyActor::ProfileAssociated& associated);

    int getLists() const { return mLists; }
    int getFeeds() const { return mFeeds; }
    bool isLabeler() const { return mLabeler; }
    const ProfileAssociatedChat& getChat() const { return mChat; }

private:
    int mLists = 0;
    int mFeeds = 0;
    bool mLabeler = false;
    ProfileAssociatedChat mChat;
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
    Q_PROPERTY(ImageView imageView READ getImageView FINAL)
    Q_PROPERTY(ProfileAssociated associated READ getAssociated FINAL)
    Q_PROPERTY(ProfileViewerState viewer READ getViewer FINAL)
    Q_PROPERTY(ContentLabelList labels READ getContentLabels FINAL)
    QML_VALUE_TYPE(basicprofile)

public:
    BasicProfile() = default;
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic* profile);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileView* profile);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewDetailed* profile);
    BasicProfile(const QString& did, const QString& handle, const QString& displayName,
                 const QString& avatarUrl, const ProfileAssociated& associated = {},
                 const ProfileViewerState& viewer = {},
                 const ContentLabelList& contentLabels = {});
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileView& profile);

    Q_INVOKABLE bool isNull() const;
    QString getDid() const;
    QString getName() const;
    QString getDisplayName() const;
    QString getHandle() const;
    QString getAvatarUrl() const;
    ImageView getImageView() const;
    ProfileAssociated getAssociated() const;
    ProfileViewerState getViewer() const;
    ContentLabelList getContentLabels() const;

    Q_INVOKABLE bool hasInvalidHandle() const;

    // Get the handle, but if it is invalid then get the DID
    QString getHandleOrDid() const;

    // The profile is volatile if it depends on pointers to the raw data.
    bool isVolatile() const;

    BasicProfile nonVolatileCopy() const;

    // Setting only makes sense for a non-volatile instance
    void setDisplayName(const QString& displayName) { mDisplayName = displayName; }

    // If avatarUrl is a "image://", then the profile takes ownership of the image
    void setAvatarUrl(const QString& avatarUrl);

    Q_INVOKABLE bool isFixedLabeler() const;
    Q_INVOKABLE bool canSendDirectMessage() const;

    // Check if communication is blocked either due to the user blocking it
    // or being blocked by it.
    Q_INVOKABLE bool isBlocked() const;

protected:
    const ATProto::AppBskyActor::ProfileViewDetailed* mProfileDetailedView = nullptr;
    const ATProto::AppBskyActor::ProfileView* mProfileView = nullptr;

private:
    const ATProto::AppBskyActor::ProfileViewBasic* mProfileBasicView = nullptr;

    QString mDid;
    QString mHandle;
    QString mDisplayName;
    QString mAvatarUrl;
    SharedImageSource::SharedPtr mAvatarSource;
    ProfileAssociated mAssociated;
    ProfileViewerState mViewer;
    ContentLabelList mContentLabels;
};

using BasicProfileList = QList<BasicProfile>;

class Profile : public BasicProfile
{
    Q_GADGET
    Q_PROPERTY(QString description READ getDescription FINAL)
    QML_VALUE_TYPE(profile)

public:
    Profile() = default;
    explicit Profile(const ATProto::AppBskyActor::ProfileView* profile);
    explicit Profile(const ATProto::AppBskyActor::ProfileViewDetailed* profile);
    explicit Profile(const ATProto::AppBskyActor::ProfileView::SharedPtr& profile);
    explicit Profile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile);
    Profile(const QString& did, const QString& handle, const QString& displayName,
            const QString& avatarUrl, const ProfileAssociated& associated,
            const ProfileViewerState& viewer,
            const ContentLabelList& contentLabels, const QString& description);

    QString getDescription() const;

    Profile nonVolatileCopy() const;

    void setDescription(const QString& description) { mDescription = description; }

private:
    ATProto::AppBskyActor::ProfileView::SharedPtr mProfile;
    ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr mDetailedProfile;

    QString mDescription;
};

class DetailedProfile : public Profile
{
    Q_GADGET
    Q_PROPERTY(QString banner READ getBanner FINAL)
    Q_PROPERTY(int followersCount READ getFollowersCount FINAL)
    Q_PROPERTY(int followsCount READ getFollowsCount FINAL)
    Q_PROPERTY(int postsCount READ getPostsCount FINAL)
    QML_VALUE_TYPE(detailedprofile)

public:
    DetailedProfile() = default;
    explicit DetailedProfile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile);
    DetailedProfile(const QString& did, const QString& handle, const QString& displayName,
                    const QString& avatarUrl, const ProfileAssociated& associated,
                    const ProfileViewerState& viewer,
                    const ContentLabelList& contentLabels, const QString& description,
                    const QString& banner, int followersCount, int followsCount, int postsCount);

    QString getBanner() const;
    int getFollowersCount() const;
    int getFollowsCount() const;
    int getPostsCount() const;

    DetailedProfile nonVolatileCopy() const;

    void setBanner(const QString& banner) { mBanner = banner; }

private:
    ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr mDetailedProfile;

    QString mBanner;
    int mFollowersCount = 0;
    int mFollowsCount = 0;
    int mPostsCount = 0;
};

}

Q_DECLARE_METATYPE(::Skywalker::ProfileViewerState)
Q_DECLARE_METATYPE(::Skywalker::ProfileAssociated)
Q_DECLARE_METATYPE(::Skywalker::BasicProfile)
Q_DECLARE_METATYPE(::Skywalker::Profile)
Q_DECLARE_METATYPE(::Skywalker::DetailedProfile)
