// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "external_source_theme.h"
#include "svg_image.h"
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QtQmlIntegration>

namespace Skywalker {

class ExternalSource
{
    Q_GADGET
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString icon READ getIcon FINAL)
    Q_PROPERTY(QString title READ getTitle FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(ExternalSourceTheme theme READ getTheme FINAL)
    QML_VALUE_TYPE(externalsource)

public:
    ExternalSource() = default;
    explicit ExternalSource(const ATProto::AppBskyEmbed::ViewExternalSource::SharedPtr& source);

    Q_INVOKABLE bool isNull() const { return mSource == nullptr; }
    QString getUri() const;
    QString getIcon() const;
    QString getTitle() const;
    QString getDescription() const;
    ExternalSourceTheme getTheme() const;
    Q_INVOKABLE QString getStandardSitePublisher();
    Q_INVOKABLE SvgImage* getStandardSitePublisherIcon();

private:
    void initPublisher();
    ATProto::AppBskyEmbed::ViewExternalSource::SharedPtr mSource;
    QString mPublisher;
    SvgImage* mPublisherIcon;
};

}
