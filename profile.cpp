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

    if (!mHandle.isEmpty())
        return mHandle;

    return {};
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
