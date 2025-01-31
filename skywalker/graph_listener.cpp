// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "graph_listener.h"

namespace Skywalker {

std::unique_ptr<GraphListener> GraphListener::sInstance;

GraphListener& GraphListener::instance()
{
    if (!sInstance)
        sInstance = std::make_unique<GraphListener>();

    return *sInstance;
}

GraphListener::GraphListener(QObject* parent) : QObject(parent)
{
}

}
