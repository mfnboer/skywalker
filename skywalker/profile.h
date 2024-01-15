// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "image_view.h"
#include "list_view_include.h"
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

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

private:
    bool mValid = false;
    bool mMuted = false;
    bool mBlockedBy = false;
    QString mBlocking;
    QString mFollowing;
    QString mFollowedBy;
    ListViewBasic mMutedByList;
    ListViewBasic mBlockingByList;
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
    Q_PROPERTY(ProfileViewerState viewer READ getViewer FINAL)
    Q_PROPERTY(QStringList labels READ getLabelTexts FINAL)
    QML_VALUE_TYPE(basicprofile)

public:
    BasicProfile() = default;
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic* profile);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileView* profile);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewDetailed* profile);
    BasicProfile(const QString& did, const QString& handle, const QString& displayName, const QString& avatarUrl);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileView& profile);

    Q_INVOKABLE bool isNull() const;
    QString getDid() const;
    QString getName() const;
    QString getDisplayName() const;
    QString getHandle() const;
    QString getAvatarUrl() const;
    ImageView getImageView() const;
    ProfileViewerState getViewer() const;
    QStringList getLabelTexts() const;

    Q_INVOKABLE bool hasInvalidHandle() const;

    // Get the handle, but if it is invalid then get the DID
    QString getHandleOrDid() const;

    // The profile is volatile if it depends on pointers to the raw data.
    bool isVolatile() const;

    BasicProfile nonVolatileCopy() const;

protected:
    const ATProto::AppBskyActor::ProfileViewDetailed* mProfileDetailedView = nullptr;
    const ATProto::AppBskyActor::ProfileView* mProfileView = nullptr;

private:
    const ATProto::AppBskyActor::ProfileViewBasic* mProfileBasicView = nullptr;

    QString mDid;
    QString mHandle;
    QString mDisplayName;
    QString mAvatarUrl;
    ProfileViewerState mViewer;
    QStringList mLabelTexts;
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

    QString getDescription() const;

private:
    ATProto::AppBskyActor::ProfileView::SharedPtr mProfile;
    ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr mDetailedProfile;
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

    QString getBanner() const;
    int getFollowersCount() const;
    int getFollowsCount() const;
    int getPostsCount() const;

private:
    ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr mDetailedProfile;
};

}

Q_DECLARE_METATYPE(Skywalker::ProfileViewerState)
Q_DECLARE_METATYPE(Skywalker::BasicProfile)
Q_DECLARE_METATYPE(Skywalker::Profile)
Q_DECLARE_METATYPE(Skywalker::DetailedProfile)
