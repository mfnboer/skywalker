// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "external_source.h"
#include "svg_outline.h"

namespace Skywalker {

ExternalSource::ExternalSource() :
    mPublisherIcon{SvgOutline::instance()->sStandardSiteIcon}
{
}

ExternalSource::ExternalSource(const ATProto::AppBskyEmbed::ViewExternalSource::SharedPtr& source) :
    mSource(source),
    mPublisherIcon{SvgOutline::instance()->sStandardSiteIcon}
{
    initPublisher();
}

QString ExternalSource::getUri() const
{
    return mSource ? mSource->mUri : "";
}

QString ExternalSource::getIcon() const
{
    return mSource ? mSource->mIcon.value_or("") : "";
}

QString ExternalSource::getTitle() const
{
    return mSource ? mSource->mTitle : "";
}

QString ExternalSource::getDescription() const
{
    return mSource ? mSource->mDescription.value_or("") : "";
}

ExternalSourceTheme ExternalSource::getTheme() const
{
    return mSource ? ExternalSourceTheme{mSource->mTheme} : ExternalSourceTheme{};
}

ATProto::AppBskyEmbed::ViewExternalSource::SharedPtr ExternalSource::getSource() const
{
    return mSource;
}

QString ExternalSource::getStandardSitePublisher() const
{
    return mPublisher;
}

SvgImage* ExternalSource::getStandardSitePublisherIcon() const
{
    return mPublisherIcon;
}

void ExternalSource::initPublisher()
{
    static const std::map<QString, std::pair<QString, SvgImage*>> STANDARD_SITE_PUBLISHERS = {
        { "leaflet.pub", { "Leaflet", SvgOutline::instance()->sLeafletIcon }},
        { "pckt.blog", { "pckt", SvgOutline::instance()->sPcktIcon }},
        { "offprint.app", { "Offprint", SvgOutline::instance()->sOffprintIcon }}
    };

    const QString uri = getUri();

    if (uri.isEmpty())
        return;

    const QString cleanedUri = uri.endsWith('/') ? uri.chopped(1) : uri;

    for (const auto& [host, publisher] : STANDARD_SITE_PUBLISHERS)
    {
        if (cleanedUri.endsWith(host))
        {
            mPublisher = publisher.first;
            mPublisherIcon = publisher.second;
            return;
        }
    }
}

}
