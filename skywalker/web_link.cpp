// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "web_link.h"
#include <atproto/lib/rich_text_master.h>
#include <QDebug>

namespace Skywalker {

WebLink::WebLink(const QString& link, int startIndex, int endIndex, const QString& name) :
    mLink(link),
    mName(name),
    mStartIndex(startIndex),
    mEndIndex(endIndex)
{
    if (!mLink.startsWith("http"))
        mLink = "https://" + mLink;

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

}
