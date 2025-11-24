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
    explicit ExternalView(const ATProto::AppBskyEmbed::ExternalViewExternal::SharedPtr& external);

    Q_INVOKABLE bool isNull() const { return mExternal == nullptr; }
    QString getUri() const;
    QString getTitle() const;
    QString getDescription() const;
    QString getThumbUrl() const;

    Q_INVOKABLE bool hasHtmlTitle() const { return !mHtmlTitle.isEmpty(); }
    void setHtmlTitle(const QString& htmlTitle) { mHtmlTitle = htmlTitle; }

    Q_INVOKABLE bool hasHtmlDescription() const { return !mHtmlDescription.isEmpty(); }
    void setHtmlDescription(const QString& htmlDescription) { mHtmlDescription = htmlDescription; }

private:
    ATProto::AppBskyEmbed::ExternalViewExternal::SharedPtr mExternal;
    QString mHtmlTitle;
    QString mHtmlDescription;
};

}

Q_DECLARE_METATYPE(::Skywalker::ExternalView)
