// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "link_utils.h"
#include "at_regex.h"
#include "skywalker.h"
#include <atproto/lib/at_regex.h>
#include <atproto/lib/tlds.h>
#include <QRegularExpression>

namespace Skywalker {

bool LinkUtils::isDomain(const QString& value)
{
    const auto index = value.lastIndexOf('.');

    if (index < 0)
        return false;

    const auto& lastPart = value.sliced(index + 1);

    if (!ATProto::isValidTLD(lastPart))
        return false;

    if (index == 0)
        return true;

    // Handles are domains!
    auto match = value.startsWith('.') ? ATProto::ATRegex::HANDLE.match(value) : ATProto::ATRegex::HANDLE.match(value.sliced(1));
    return match.hasMatch();
}

LinkUtils::LinkUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

bool LinkUtils::isWebLink(const QString& link)
{
    if (link.contains(' '))
        return false;

    const QUrl url(link);
    return url.isValid() && (url.scheme() == "https" || url.scheme() == "http");
}

bool LinkUtils::hasScheme(const QString& link)
{
    const QUrl url(link);
    return !url.scheme().isEmpty();
}

QString LinkUtils::toHttpsLink(const QString& atUri)
{
    ATProto::ATUri uri(atUri);

    if (!uri.isValid())
        return {};

    const QString httpsLink = uri.toHttpsUri();
    Q_ASSERT(!httpsLink.isEmpty());
    return httpsLink;
}

void LinkUtils::openLink(const QString& link, const QString& containingText)
{
    const auto atUri = getPostUri(link);

    qDebug() << "LinkUtils is opening: " << link << " with text " << containingText;

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

    const auto listAtUri = getListUri(link);

    if (listAtUri.isValid())
    {
        openListLink(listAtUri);
        return;
    }

    const auto starterPackAtUri = getStarterPackUri(link);

    if (starterPackAtUri.isValid())
    {
        openStarterPackLink(starterPackAtUri);
        return;
    }

    const auto handleOrDid = isAuthorLink(link);

    if (!handleOrDid.isEmpty())
    {
        emit authorLink(handleOrDid);
        return;
    }

    const QUrl url(link);
    bool hostPresent = url.isValid() && containingText.contains(url.host(), Qt::CaseInsensitive);
    qDebug() << "LinkUtils is emitting webLink with hostPresent: " << hostPresent;
    emit webLink(link, containingText, hostPresent);
}

void LinkUtils::openLink(const ATProto::ATUri& atUri, const std::function<void(const QString& uri)>& openFun)
{
    if (!atUri.authorityIsHandle())
    {
        openFun(atUri.toString());
        return;
    }

    bskyClient()->resolveHandle(atUri.getAuthority(),
        [presence=getPresence(), atUri, openFun](const QString& did){
            if (!presence)
                return;

            auto postAtUri = atUri;
            postAtUri.setAuthority(did);
            postAtUri.setAuthorityIsHandle(false);
            openFun(postAtUri.toString());
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << error << " - " << msg;
            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void LinkUtils::openPostLink(const ATProto::ATUri& atUri)
{
    openLink(atUri, [this](const QString& uri){ emit postLink(uri); });
}

void LinkUtils::openFeedLink(const ATProto::ATUri& atUri)
{
    openLink(atUri, [this](const QString& uri){ emit feedLink(uri); });
}

void LinkUtils::openListLink(const ATProto::ATUri& atUri)
{
    openLink(atUri, [this](const QString& uri){ emit listLink(uri); });
}

void LinkUtils::openStarterPackLink(const ATProto::ATUri& atUri)
{
    openLink(atUri, [this](const QString& uri){ emit starterPackLink(uri); });
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

ATProto::ATUri LinkUtils::getListUri(const QString& link) const
{
    const auto atUri = ATProto::ATUri::fromHttpsListUri(link);
    if (atUri.isValid())
        return atUri;

    return {};
}

ATProto::ATUri LinkUtils::getStarterPackUri(const QString& link) const
{
    const auto atUri = ATProto::ATUri::fromHttpsStarterPackUri(link);
    if (atUri.isValid())
        return atUri;

    return {};
}

}
