// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QtQmlIntegration>

namespace Skywalker
{

class ExternalView
{
    Q_GADGET
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString title READ getTitle FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(QString thumbUrl READ getThumbUrl FINAL)
    QML_VALUE_TYPE(externalview)

public:
    using Ptr = std::unique_ptr<ExternalView>;

    ExternalView() = default;
    explicit ExternalView(const ATProto::AppBskyEmbed::ExternalViewExternal* external);

    QString getUri() const { return mUri; }
    QString getTitle() const { return mTitle; }
    QString getDescription() const { return mDescription; }
    QString getThumbUrl() const { return mThumbUrl; }

private:
    QString mUri;
    QString mTitle;
    QString mDescription;
    QString mThumbUrl;
};

}

Q_DECLARE_METATYPE(::Skywalker::ExternalView)
