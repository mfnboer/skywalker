// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "web_link.h"
#include <QTextBoundaryFinder>

namespace Skywalker {

class SplitTextBoundaryFinder
{
public:
    SplitTextBoundaryFinder(const QString& text, const WebLink::List& embeddedLinks);

    QTextBoundaryFinder::BoundaryReasons boundaryReasons() const { return mBoundaryFinder.boundaryReasons(); }
    int toNextBoundary();

private:
    bool isInsideEmbeddedLinks(int index) const;

    QTextBoundaryFinder mBoundaryFinder;
    const WebLink::List& mEmbeddedLinks;
};

}
