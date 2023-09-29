// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile.h"

namespace Skywalker {

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

BasicProfile::BasicProfile(const QString& did, const QString& handle, const QString& displayName, const QString& avatarUrl) :
    mDid(did),
    mHandle(handle),
    mDisplayName(displayName),
    mAvatarUrl(avatarUrl)
{
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileView& profile) :
    mDid(profile.mDid),
    mHandle(profile.mHandle),
    mDisplayName(profile.mDisplayName.value_or(QString())),
    mAvatarUrl(profile.mAvatar.value_or(QString()))
{
}

QString BasicProfile::getDid() const
{
    return mProfileBasicView ? mProfileBasicView->mDid :
               mProfileView ? mProfileView->mDid : mDid;
}

static QString createName(const QString& handle, const std::optional<QString>& displayName)
{
    const QString name = displayName.value_or("").trimmed();
    return name.isEmpty() ? handle : name;
}

QString BasicProfile::getName() const
{
    return createName(getHandle(), getDisplayName());
}

std::optional<QString> BasicProfile::getDisplayName() const
{
    return mProfileBasicView ? mProfileBasicView->mDisplayName :
               mProfileView? mProfileView->mDisplayName : mDisplayName;
}

QString BasicProfile::getHandle() const
{
    return mProfileBasicView ? mProfileBasicView->mHandle :
               mProfileView? mProfileView->mHandle : mHandle;
}

QString BasicProfile::getAvatarUrl() const
{
    return (mProfileBasicView && mProfileBasicView->mAvatar) ? *mProfileBasicView->mAvatar :
               (mProfileView && mProfileView->mAvatar) ? *mProfileView->mAvatar : mAvatarUrl;
}

bool BasicProfile::isVolatile() const
{
    return mProfileBasicView || mProfileView;
}

BasicProfile BasicProfile::nonVolatileCopy() const
{
    BasicProfile profile(mDid, mHandle, mDisplayName, mAvatarUrl);
    return profile;
}

CachedBasicProfile::CachedBasicProfile(const BasicProfile& profile) :
    QObject(),
    mProfile(profile)
{
}

}
