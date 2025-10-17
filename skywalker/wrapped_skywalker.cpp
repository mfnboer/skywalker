// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "wrapped_skywalker.h"
#include "skywalker.h"

namespace Skywalker {

WrappedSkywalker::WrappedSkywalker(QObject* parent) :
    QObject(parent)
{
}

void WrappedSkywalker::setSkywalker(Skywalker* skywalker)
{
    Q_ASSERT(skywalker);

    if (skywalker != mSkywalker)
    {
        mSkywalker = skywalker;
        emit skywalkerChanged();
    }
}

void WrappedSkywalker::setNonActiveUserDid(const QString& did)
{
    if (did != mNonActiveUserDid)
    {
        mNonActiveUserDid = did;
        emit nonActiveUserDidChanged();

        // Get a shared reference to the client now. It may expire during the lifetme
        // of this object. This way we keep it alive. Requests will fail, but that
        // is fine, the session is expired after all.
        mNonActiveUserBsky = mSkywalker->getSessionManager()->getBskyClientFor(mNonActiveUserDid);

        // There is a non-zero chance it has already been expired (e.g. the user clicks on
        // reply on behalf of a non-active user. The session expires while Qt creates the
        // post composition page.
        if (!mNonActiveUserBsky)
        {
            qDebug() << "Session already expired:" << did;
            emit nonActiveUserSessionExpired();
        }
    }
}

// The client for the active user gets never destroyed. It will be
// reconfigured on user switching.
// Clients for non-active users can be destroyed, e.g. on refresh
// failure. Hence the shared ptr to keep it alive till it is not
// needed anymore.
ATProto::Client* WrappedSkywalker::bskyClient()
{
    Q_ASSERT(mSkywalker);
    ATProto::Client* client = nullptr;

    if (mNonActiveUserDid.isEmpty())
    {
        client = mSkywalker->getBskyClient();
    }
    else
    {
        if (!mNonActiveUserBsky)
            mNonActiveUserBsky = mSkywalker->getSessionManager()->getBskyClientFor(mNonActiveUserDid);

        client = mNonActiveUserBsky.get();
    }

    Q_ASSERT(client);
    return client;
}

ATProto::PlcDirectoryClient& WrappedSkywalker::plcDirectory()
{
    Q_ASSERT(mSkywalker);
    return mSkywalker->getPlcDirectory();
}

}
