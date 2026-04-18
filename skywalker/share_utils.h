// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include "list_view.h"
#include "starter_pack.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class ShareUtils : public WrappedSkywalker
{
    Q_OBJECT
public:
    explicit ShareUtils(QObject* parent = nullptr);

    Q_INVOKABLE void sharePost(const QString& postUri);
    Q_INVOKABLE void shareFeed(const GeneratorView& feed);
    Q_INVOKABLE void shareList(const ListView& list);
    Q_INVOKABLE void shareStarterPack(const StarterPackViewBasic& starterPack);
    Q_INVOKABLE void shareAuthor(const BasicProfile& author);
    void openLinkInApp(const QString& link);
    Q_INVOKABLE void copyPostTextToClipboard(const QString& text);
    Q_INVOKABLE void copyToClipboard(const QString& text);
};

}
