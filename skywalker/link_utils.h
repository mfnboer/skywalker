// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

class LinkUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    Q_INVOKABLE static bool isDomain(const QString& value);
    Q_INVOKABLE static bool isWebLink(const QString& link);
    Q_INVOKABLE static bool hasScheme(const QString& link);
    Q_INVOKABLE static bool isValidDid(const QString& did);
    Q_INVOKABLE static bool isValidService(const QString& service); // atproto-proxy header value
    Q_INVOKABLE static QString getLinkWithScheme(const QString& link);

    explicit LinkUtils(QObject* parent = nullptr);

    Q_INVOKABLE QString toHttpsLink(const QString& atUri);
    Q_INVOKABLE void openLink(const QString& link, const QString& containingText = {});

signals:
    void webLink(QString uri, QString containingText, bool hostInContainingText);
    void authorLink(QString handleOrDid);
    void postLink(QString atUri);
    void feedLink(QString atUri);
    void listLink(QString atUri);
    void starterPackLink(QString atUri);

private:
    void openLink(const ATProto::ATUri& atUri, const std::function<void(const QString& uri)>& openFun);
    void openPostLink(const ATProto::ATUri& atUri);
    void openFeedLink(const ATProto::ATUri& atUri);
    void openListLink(const ATProto::ATUri& atUri);
    void openStarterPackLink(const ATProto::ATUri& atUri);
    QString isAuthorLink(const QString& link) const;
    ATProto::ATUri getPostUri(const QString& link) const;
    ATProto::ATUri getFeedUri(const QString& link) const;
    ATProto::ATUri getListUri(const QString& link) const;
    ATProto::ATUri getStarterPackUri(const QString& link) const;
};

}
