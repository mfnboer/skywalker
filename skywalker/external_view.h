// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "external_source.h"
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
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(QDateTime updateAt READ getUpdatedAt FINAL)
    Q_PROPERTY(int readingTime READ getReadingTime FINAL)
    Q_PROPERTY(ContentLabelList contentLabels READ getContentLabels FINAL)
    Q_PROPERTY(ExternalSource source READ getSource FINAL)
    Q_PROPERTY(QVariant associatedProfiles READ getAssociatedProfiles FINAL)
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
    QDateTime getCreatedAt() const;
    QDateTime getUpdatedAt() const;
    int getReadingTime() const; // minutes
    ContentLabelList getContentLabels() const;
    ExternalSource getSource() const;
    QVariant getAssociatedProfiles() const; // return QVariant to break cyclic dependency on profile.h

    Q_INVOKABLE bool hasHtmlTitle() const { return !mHtmlTitle.isEmpty(); }
    void setHtmlTitle(const QString& htmlTitle) { mHtmlTitle = htmlTitle; }

    Q_INVOKABLE bool hasHtmlDescription() const { return !mHtmlDescription.isEmpty(); }
    void setHtmlDescription(const QString& htmlDescription) { mHtmlDescription = htmlDescription; }

    ATProto::AppBskyEmbed::ExternalViewExternal::SharedPtr getExternal() const { return mExternal; }

private:
    ATProto::AppBskyEmbed::ExternalViewExternal::SharedPtr mExternal;
    QString mHtmlTitle;
    QString mHtmlDescription;
};

}

Q_DECLARE_METATYPE(::Skywalker::ExternalView)
