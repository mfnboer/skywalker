// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class BasicProfile
{
    Q_GADGET
    Q_PROPERTY(QString did READ getDid FINAL)
    Q_PROPERTY(QString handle READ getHandle FINAL)
    Q_PROPERTY(QString displayName READ getDisplayName FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(QString avatarUrl READ getAvatarUrl FINAL)
    QML_VALUE_TYPE(basicprofile)

public:
    BasicProfile() = default;
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic* profile);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileView* profile);
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewDetailed* profile);
    BasicProfile(const QString& did, const QString& handle, const QString& displayName, const QString& avatarUrl);
    BasicProfile(const ATProto::AppBskyActor::ProfileView& profile);

    QString getDid() const;
    QString getName() const;
    QString getDisplayName() const;
    QString getHandle() const;
    QString getAvatarUrl() const;

    // The profile is volatile if it depends on pointers to the raw data.
    bool isVolatile() const;

    BasicProfile nonVolatileCopy() const;

protected:
    const ATProto::AppBskyActor::ProfileViewDetailed* mProfileDetailedView = nullptr;

private:
    const ATProto::AppBskyActor::ProfileViewBasic* mProfileBasicView = nullptr;
    const ATProto::AppBskyActor::ProfileView* mProfileView = nullptr;
    QString mDid;
    QString mHandle;
    QString mDisplayName;
    QString mAvatarUrl;
};

class DetailedProfile : public BasicProfile
{
    Q_GADGET
    Q_PROPERTY(QString banner READ getBanner FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(int followersCount READ getFollowersCount FINAL)
    Q_PROPERTY(int followsCount READ getFollowsCount FINAL)
    Q_PROPERTY(int postsCount READ getPostsCount FINAL)
    QML_VALUE_TYPE(detailedprofile)

public:
    DetailedProfile() = default;
    DetailedProfile(const ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr& profile);

    QString getBanner() const;
    QString getDescription() const;
    int getFollowersCount() const;
    int getFollowsCount() const;
    int getPostsCount() const;

private:
    ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr mDetailedProfile;
};

}

Q_DECLARE_METATYPE(Skywalker::BasicProfile)
Q_DECLARE_METATYPE(Skywalker::DetailedProfile)
