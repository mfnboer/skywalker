// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "link_utils.h"
#include "at_regex.h"
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

    const auto feedAtUri = getFeedUri(link);

    if (feedAtUri.isValid())
    {
        openFeedLink(feedAtUri);
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
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << error << " - " << msg;
            mSkywalker->statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void LinkUtils::openFeedLink(const ATProto::ATUri& atUri)
{
    if (!atUri.authorityIsHandle())
    {
        emit feedLink(atUri.toString());
        return;
    }

    bskyClient()->resolveHandle(atUri.getAuthority(),
        [this, presence=getPresence(), atUri](const QString& did){
            if (!presence)
                return;

            auto feedAtUri = atUri;
            feedAtUri.setAuthority(did);
            feedAtUri.setAuthorityIsHandle(false);
            emit feedLink(feedAtUri.toString());
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << error << " - " << msg;
            mSkywalker->statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

QString LinkUtils::isAuthorLink(const QString& link) const
{
    static const QRegularExpression authorHandleRE(
            QString(R"(^https:\/\/bsky.app\/profile\/(?<handle>%1)$)").arg(
                ATProto::ATRegex::HANDLE.pattern()));
    static const QRegularExpression authorDidRE(
            QString(R"(^https:\/\/bsky.app\/profile\/(?<did>%2)$)").arg(
                ATProto::ATRegex::DID.pattern()));

    auto match = authorHandleRE.match(link);
    if (match.hasMatch())
        return match.captured("handle");

    match = authorDidRE.match(link);
    if (match.hasMatch())
        return match.captured("did");

    return {};
}

ATProto::ATUri LinkUtils::getPostUri(const QString& link) const
{
    const auto atUri = ATProto::ATUri::fromHttpsPostUri(link);
    if (atUri.isValid())
        return atUri;

    return {};
}

ATProto::ATUri LinkUtils::getFeedUri(const QString& link) const
{
    const auto atUri = ATProto::ATUri::fromHttpsFeedUri(link);
    if (atUri.isValid())
        return atUri;

    return {};
}

}
