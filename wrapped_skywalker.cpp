// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "wrapped_skywalker.h"

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

ATProto::Client* WrappedSkywalker::bskyClient()
{
    Q_ASSERT(mSkywalker);
    auto* client = mSkywalker->getBskyClient();
    Q_ASSERT(client);
    return client;
}

}
