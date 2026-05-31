// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "external_source.h"
#include "svg_outline.h"

namespace Skywalker {

ExternalSource::ExternalSource(const ATProto::AppBskyEmbed::ViewExternalSource::SharedPtr& source) :
    mSource(source)
{
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

QString ExternalSource::getStandardSitePublisher()
{
    initPublisher();
    return mPublisher;
}

SvgImage* ExternalSource::getStandardSitePublisherIcon()
{
    initPublisher();
    return mPublisherIcon;
}

void ExternalSource::initPublisher()
{
    if (!mPublisher.isNull())
        return;

    static const std::map<QString, std::pair<QString, SvgImage*>> STANDARD_SITE_PUBLISHERS = {
        { "leaflet.pub", { "Leaflet", SvgOutline::instance()->sLeafletIcon }},
        { "pckt.blog", { "pckt", SvgOutline::instance()->sPcktIcon }},
        { "offprint.app", { "Offprint", SvgOutline::instance()->sOffprintIcon }}
    };

    const QString uri = getUri();
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

    mPublisher = "";
    mPublisherIcon = SvgOutline::instance()->sStandardSiteIcon;
}

}
