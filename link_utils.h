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
    explicit LinkUtils(QObject* parent = nullptr);

    Q_INVOKABLE void openLink(const QString& link);

signals:
    void webLink(QString uri);
    void authorLink(QString handle);
    void postLink(QString atUri);

private:
    void openPostLink(const ATProto::ATUri& atUri);
    QString isAuthorLink(const QString& link) const;
    ATProto::ATUri getPostUri(const QString& link) const;
};

}
