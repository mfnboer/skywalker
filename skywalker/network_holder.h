// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QNetworkAccessManager>

namespace Skywalker {

class NetworkHolder
{
public:
    static QNetworkAccessManager* network();

private:
    static std::unique_ptr<QNetworkAccessManager> sNetwork;
};

}