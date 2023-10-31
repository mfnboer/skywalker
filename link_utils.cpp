// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "link_utils.h"
#include <QRegularExpression>

namespace Skywalker {

LinkUtils::LinkUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void LinkUtils::openLink(const QString& link)
{
    const auto atUri = getPostUri(link);

    if (atUri.isValid())
    {
        openPostLink(atUri);
        return;
    }

    const auto handleOrDid = isAuthorLink(link);

    if (!handleOrDid.isEmpty())
    {
        emit authorLink(handleOrDid);
        return;
    }

    emit webLink(link);
}

void LinkUtils::openPostLink(const ATProto::ATUri& atUri)
{
    if (!atUri.authorityIsHandle())
    {
        emit postLink(atUri.toString());
        return;
    }

    bskyClient()->resolveHandle(atUri.getAuthority(),
        [this, presence=getPresence(), atUri](const QString& did){
            if (!presence)
                return;

            auto postAtUri = atUri;
            postAtUri.setAuthority(did);
            postAtUri.setAuthorityIsHandle(false);
            emit postLink(postAtUri.toString());
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qWarning() << error;
            mSkywalker->statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
}

QString LinkUtils::isAuthorLink(const QString& link) const
{
    static const QRegularExpression authorHandleRE(R"(^https:\/\/bsky.app\/profile\/([a-zA-Z0-9-\._~]+)$)");
    static const QRegularExpression authorDidRE(R"(^https:\/\/bsky.app\/profile\/(did:plc:[a-zA-Z0-9-\.:_]+)$)");

    auto match = authorHandleRE.match(link);
    if (match.hasMatch())
        return match.captured(1);

    match = authorDidRE.match(link);
    if (match.hasMatch())
        return match.captured(1);

    return {};
}

ATProto::ATUri LinkUtils::getPostUri(const QString& link) const
{
    const auto atUri = ATProto::ATUri::fromHttpsPostUri(link);
    if (atUri.isValid())
        return atUri;

    return {};
}

}
