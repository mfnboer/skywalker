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
    }
}

// TODO: what if the bsky client gets destroyed due to a failing session refresh?
// Maybe we should not detroy the client, such that the pointer stays alive. Request
// will fail, but that is fine. Or use a shared pointer to keep it alive until it is
// not needed here anymore?
ATProto::Client* WrappedSkywalker::bskyClient()
{
    Q_ASSERT(mSkywalker);
    auto* client = mNonActiveUserDid.isEmpty() ?
                       mSkywalker->getBskyClient() :
                       mSkywalker->getSessionManager()->getBskyClientFor(mNonActiveUserDid);
    Q_ASSERT(client);
    return client;
}

ATProto::PlcDirectoryClient& WrappedSkywalker::plcDirectory()
{
    Q_ASSERT(mSkywalker);
    return mSkywalker->getPlcDirectory();
}

}
