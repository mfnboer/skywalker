// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile.h"

namespace Skywalker {

BasicProfile::BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic* profile) :
    mProfile(profile)
{
    Q_ASSERT(mProfile);
}

QString BasicProfile::getName() const
{
    const QString name = mProfile->mDisplayName.value_or("").trimmed();
    return name.isEmpty() ? mProfile->mHandle : name;
}

QString BasicProfile::getAvatarUrl() const
{
    return mProfile->mAvatar ? *mProfile->mAvatar : QString();
}

}
