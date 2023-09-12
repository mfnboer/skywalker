// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile.h"

namespace Skywalker {

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic* profile) :
    mProfile(profile)
{
    Q_ASSERT(mProfile);
}

BasicProfile::BasicProfile(const QString& handle, const QString& displayName) :
    mHandle(handle),
    mDisplayName(displayName)
{
}

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileView& profile) :
    mDid(profile.mDid),
    mHandle(profile.mHandle),
    mDisplayName(profile.mDisplayName.value_or(QString()))
{
}

QString BasicProfile::getDid() const
{
    return mProfile ? mProfile->mDid : mDid;
}

QString BasicProfile::getName() const
{
    if (mProfile)
    {
        const QString name = mProfile->mDisplayName.value_or("").trimmed();
        return name.isEmpty() ? mProfile->mHandle : name;
    }

    const QString name = mDisplayName.trimmed();
    if (!name.isEmpty())
        return name.trimmed();

    return mHandle;
}

QString BasicProfile::getHandle() const
{
    return mProfile ? mProfile->mHandle : mHandle;
}

QString BasicProfile::getAvatarUrl() const
{
    return (mProfile && mProfile->mAvatar) ? *mProfile->mAvatar : QString();
}

CachedBasicProfile::CachedBasicProfile(const BasicProfile& profile) :
    QObject(),
    mProfile(profile)
{
}

}
