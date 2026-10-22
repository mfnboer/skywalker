// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "network_holder.h"

namespace Skywalker {

std::unique_ptr<QNetworkAccessManager> NetworkHolder::sNetwork;

QNetworkAccessManager* NetworkHolder::network()
{
    if (!sNetwork)
    {
        sNetwork = std::make_unique<QNetworkAccessManager>();
        sNetwork->setAutoDeleteReplies(true);
        sNetwork->setTransferTimeout(10000);
    }

    return sNetwork.get();
}

}