// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "basic_profile_view.h"

namespace Skywalker {

BasicProfileView::BasicProfileView(QObject* parent) :
    QObject(parent)
{
}

BasicProfileView::BasicProfileView(const BasicProfile& profile, QObject* parent) :
    QObject(parent),
    mProfile(profile),
    mDid(profile.getDid())
{
}

BasicProfileView::BasicProfileView(const QString& did, const QString& error, QObject* parent) :
    QObject(parent),
    mDid(did),
    mError(error)
{
}

QString BasicProfileView::getFollowingUri() const
{
    if (mFollowingUri)
        return *mFollowingUri;

    return mProfile.getViewer().getFollowing();
}

void BasicProfileView::setFollowingUri(const QString& followingUri)
{
    if (!mFollowingUri || *mFollowingUri != followingUri)
    {
        mFollowingUri = followingUri;
        emit followingUriChanged();
    }
}

}