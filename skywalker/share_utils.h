// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "convo_view.h"
#include "generator_view.h"
#include "list_view.h"
#include "search_options.h"
#include "starter_pack.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class ShareUtils : public WrappedSkywalker
{
    Q_OBJECT
public:
    explicit ShareUtils(QObject* parent = nullptr);

    Q_INVOKABLE void sharePost(const QString& postUri, const QString& feedDid = {}, const QString& feedContext = {});
    Q_INVOKABLE void shareFeed(const GeneratorView& feed);
    Q_INVOKABLE void shareList(const ListView& list);
    Q_INVOKABLE void shareListUri(const QString& listUri);
    Q_INVOKABLE void shareStarterPack(const StarterPackViewBasic& starterPack);
    Q_INVOKABLE void shareAuthor(const BasicProfile& author);
    Q_INVOKABLE void shareConvo(const ConvoView& convo);
    Q_INVOKABLE void shareSearch(const QString& searchText, const SearchOptions& searchOptions);
    void openLinkInApp(const QString& link);
    Q_INVOKABLE void copyPostTextToClipboard(const QString& text);
    Q_INVOKABLE void copyToClipboard(const QString& text);
    Q_INVOKABLE void copyUriToClipboard(const QString& uri);
    Q_INVOKABLE void copyAuthorLinkToClipboard(const BasicProfile& author);

private:
    void share(const QString& uri, const QString& subject);
};

}
