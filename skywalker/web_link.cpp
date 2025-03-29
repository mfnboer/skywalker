// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "web_link.h"
#include <atproto/lib/xjson.h>
#include <QDebug>

namespace Skywalker {

WebLink::WebLink(const QString& link, int startIndex, int endIndex, const QString& name) :
    mLink(link),
    mName(name),
    mStartIndex(startIndex),
    mEndIndex(endIndex)
{
    checkMisleadingName();
}

void WebLink::setName(const QString& name)
{
    mName = name;
    checkMisleadingName();
}

void WebLink::addToIndexes(int add)
{
    mStartIndex += add;
    mEndIndex += add;
}

bool WebLink::isValidEmbeddedLink() const
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

void WebLink::checkMisleadingName()
{
    const auto facets = ATProto::RichTextMaster::parseFacets(mName);
    mHasMisleadingName = !facets.empty();
}

QString WebLink::getMisleadingNameError() const
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

ATProto::RichTextMaster::ParsedMatch WebLink::toFacet() const
{   
    ATProto::RichTextMaster::ParsedMatch facet;
    facet.mStartIndex = mStartIndex;
    facet.mEndIndex = mEndIndex;
    facet.mMatch = mName;
    facet.mType = ATProto::RichTextMaster::ParsedMatch::Type::LINK;
    facet.mRef = mLink.startsWith("http") ? mLink : "https://" + mLink;
    return facet;
}

std::vector<ATProto::RichTextMaster::ParsedMatch> WebLink::toFacetList(const List& links)
{
    std::vector<ATProto::RichTextMaster::ParsedMatch> facets;

    for (const auto& link : links)
        facets.push_back(link.toFacet());

    return facets;
}

WebLink WebLink::fromFacet(const ATProto::RichTextMaster::ParsedMatch& facet)
{
    return WebLink(facet.mRef, facet.mStartIndex, facet.mEndIndex, facet.mMatch);
}

WebLink::List WebLink::fromFacetList(const std::vector<ATProto::RichTextMaster::ParsedMatch>& facets)
{
    List links;

    for (const auto& facet : facets)
        links.push_back(fromFacet(facet));

    return links;
}

QJsonObject WebLink::toJson() const
{
    QJsonObject json;
    json.insert("name", mName);
    json.insert("link", mLink);
    json.insert("startIndex", mStartIndex);
    json.insert("endIndex", mEndIndex);
    return json;
}

WebLink::SharedPtr WebLink::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto webLink = std::make_shared<WebLink>();
    webLink->mName = xjson.getRequiredString("name");
    webLink->mLink = xjson.getRequiredString("link");
    webLink->mStartIndex = xjson.getRequiredInt("startIndex");
    webLink->mEndIndex = xjson.getRequiredInt("endIndex");
    webLink->checkMisleadingName();
    return webLink;
}

}
