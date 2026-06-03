// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "named_link.h"
#include "author_cache.h"
#include <atproto/lib/xjson.h>
#include <QDebug>

namespace Skywalker {

NamedLink::NamedLink(QEnums::LinkType linkType, const QString& link, int startIndex, int endIndex,
                     const QString& name) :
    mLinkType(linkType),
    mLink(link),
    mName(name),
    mStartIndex(startIndex),
    mEndIndex(endIndex)
{
    checkMisleadingName();
}

void NamedLink::setName(const QString& name)
{
    mName = name;
    checkMisleadingName();
}

void NamedLink::addToIndexes(int add)
{
    mStartIndex += add;
    mEndIndex += add;
}

bool NamedLink::isValidEmbeddedLink() const
{
    if (getName().isEmpty())
    {
        qWarning() << "Embedded link must have name:" << getLink();
        return false;
    }

    if (getStartIndex() < 0 || getEndIndex() < 0)
    {
        qWarning() << "Embedded link must have indices:" << getLink() << "start:" << getStartIndex() << "end:" << getEndIndex();
        return false;
    }

    if (getStartIndex() > getEndIndex())
    {
        qWarning() << "Invalid indices:" << getLink() << "start:" << getStartIndex() << "end:" << getEndIndex();
        return false;
    }

    return true;
}

void NamedLink::checkMisleadingName()
{
    const auto facets = ATProto::RichTextMaster::parseFacets(mName);
    mHasMisleadingName = !facets.empty();
}

QString NamedLink::getMisleadingNameError() const
{
    const auto facets = ATProto::RichTextMaster::parseFacets(mName);

    if (facets.empty())
        return {};

    const auto& facet = facets.front();

    switch (facet.mType)
    {
    case ATProto::RichTextMaster::ParsedMatch::Type::LINK:
        return QObject::tr("Do not use a link url as link display name: %1").arg(facet.mMatch);
    case ATProto::RichTextMaster::ParsedMatch::Type::PARTIAL_MENTION:
    case ATProto::RichTextMaster::ParsedMatch::Type::MENTION:
        return QObject::tr("Do not use a mention as link display name: %1").arg(facet.mMatch);
    case ATProto::RichTextMaster::ParsedMatch::Type::TAG:
        return QObject::tr("Do not use a hashtag as link display name: %1").arg(facet.mMatch);
    case ATProto::RichTextMaster::ParsedMatch::Type::UNKNOWN:
        return QObject::tr("Misleading part in link display name: %1").arg(facet.mMatch);
    }

    qWarning() << "Unknown facet type:" << (int)facet.mType;
    return QObject::tr("Misleading link display name: %1").arg(facet.mMatch);
}

void NamedLink::resolveDidLinkToHandle(const std::function<void()>& doneCb)
{
    qDebug() << "Resolve:" << mLink << "name:" << mName;

    if (mLinkType != QEnums::LINK_TYPE_MENTION || !mLink.startsWith("did:"))
    {
        QTimer::singleShot(0, [doneCb]{ doneCb(); });
        return;
    }

    auto& authorCache = AuthorCache::instance();
    const BasicProfile* profile = authorCache.get(mLink);

    if (profile)
    {
        mLink = "@" + profile->getHandle();
        QTimer::singleShot(0, [doneCb]{ doneCb(); });
        return;
    }

    authorCache.putProfile(mLink, [this, presence=getPresence(), doneCb]{
        if (!presence)
            return;

        const BasicProfile* profile = AuthorCache::instance().get(mLink);

        if (profile)
            mLink = "@" + profile->getHandle();
        else
            qWarning() << "Failed to resolve DID:" << mLink;

        doneCb();
    });
}

void NamedLink::resolveDidLinksToHandles(List& links, const std::function<void()>& doneCb, int linkIndex)
{
    qDebug() << "Resolve DIDs to handles, index:" << linkIndex << "count:" << links.size();

    for (int i = linkIndex; i < links.size(); ++i)
    {
        auto& link = links[i];
        qDebug() << "Link:" << link.mLink << "name:" << link.mName << "type:" << link.mLinkType;

        if (link.mLinkType != QEnums::LINK_TYPE_MENTION)
            continue;

        link.resolveDidLinkToHandle([&links, doneCb, i]{
            resolveDidLinksToHandles(links, doneCb, i+1);
            return;
        });
    }

    doneCb();
}

ATProto::RichTextMaster::ParsedMatch NamedLink::toFacet() const
{   
    ATProto::RichTextMaster::ParsedMatch facet;
    facet.mStartIndex = mStartIndex;
    facet.mEndIndex = mEndIndex;
    facet.mMatch = mName;
    facet.mType = mLinkType == QEnums::LINK_TYPE_MENTION ?
            ATProto::RichTextMaster::ParsedMatch::Type::MENTION :
            ATProto::RichTextMaster::ParsedMatch::Type::LINK;
    facet.mRef = mLinkType == QEnums::LINK_TYPE_MENTION ?
            mLink :
            (mLink.startsWith("http") ? mLink : "https://" + mLink);
    return facet;
}

std::vector<ATProto::RichTextMaster::ParsedMatch> NamedLink::toFacetList(const List& links)
{
    std::vector<ATProto::RichTextMaster::ParsedMatch> facets;

    for (const auto& link : links)
        facets.push_back(link.toFacet());

    return facets;
}

NamedLink NamedLink::fromFacet(const ATProto::RichTextMaster::ParsedMatch& facet)
{
    const auto linkType = facet.mType == ATProto::RichTextMaster::ParsedMatch::Type::MENTION ?
            QEnums::LINK_TYPE_MENTION :
            QEnums::LINK_TYPE_WEB;

    return NamedLink(linkType, facet.mRef, facet.mStartIndex, facet.mEndIndex, facet.mMatch);
}

NamedLink::List NamedLink::fromFacetList(const std::vector<ATProto::RichTextMaster::ParsedMatch>& facets)
{
    List links;

    for (const auto& facet : facets)
        links.push_back(fromFacet(facet));

    return links;
}

QJsonObject NamedLink::toJson() const
{
    QJsonObject json;
    json.insert("linkType", (int)mLinkType);
    json.insert("name", mName);
    json.insert("link", mLink);
    json.insert("startIndex", mStartIndex);
    json.insert("endIndex", mEndIndex);
    return json;
}

NamedLink::SharedPtr NamedLink::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto webLink = std::make_shared<NamedLink>();
    webLink->mLinkType = (QEnums::LinkType)xjson.getOptionalInt("linkType", QEnums::LinkType::LINK_TYPE_WEB);
    webLink->mName = xjson.getRequiredString("name");
    webLink->mLink = xjson.getRequiredString("link");
    webLink->mStartIndex = xjson.getRequiredInt("startIndex");
    webLink->mEndIndex = xjson.getRequiredInt("endIndex");
    webLink->checkMisleadingName();
    return webLink;
}

}
