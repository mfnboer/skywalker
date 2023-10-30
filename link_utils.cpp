// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "link_utils.h"
#include <atproto/lib/at_uri.h>
#include <QRegularExpression>

namespace Skywalker {

LinkUtils::LinkUtils(QObject* parent) :
    QObject(parent)
{
}

void LinkUtils::openLink(const QString& link)
{
    const auto pl = isPostLink(link);

    if (!pl.isEmpty())
    {
        emit postLink(pl);
        return;
    }

    const auto handle = isAuthorLink(link);

    if (!handle.isEmpty())
    {
        emit authorLink(handle);
        return;
    }

    emit webLink(link);
}

QString LinkUtils::isAuthorLink(const QString& link) const
{
    static const QRegularExpression authorRE(R"(^https:\/\/bsky.app\/profile\/([a-zA-Z0-9-\._~]+)$)");

    const auto match = authorRE.match(link);
    if (match.hasMatch())
        return match.captured(1);

    return {};
}

QString LinkUtils::isPostLink(const QString& link) const
{
    const auto atUri = ATProto::ATUri::fromHttpsPostUri(link);

    // TODO: resolve handle to did
    if (atUri.isValid())
        return atUri.toString();

    return {};
}

}
