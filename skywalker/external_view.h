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

    Q_INVOKABLE bool isNull() { return mExternal == nullptr; }
    QString getUri() const;
    QString getTitle() const;
    QString getDescription() const;
    QString getThumbUrl() const;

    void setTitle(const QString& title) { mTitle = title; }
    void setDescription(const QString& description) { mDescription = description; }

private:
    ATProto::AppBskyEmbed::ExternalViewExternal::SharedPtr mExternal;
    QString mTitle;
    QString mDescription;
};

}

Q_DECLARE_METATYPE(::Skywalker::ExternalView)
