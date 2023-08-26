// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>

namespace Skywalker
{

class ExternalView
{
    Q_GADGET
    Q_PROPERTY(QString uri MEMBER mUri CONSTANT)
    Q_PROPERTY(QString title MEMBER mTitle CONSTANT)
    Q_PROPERTY(QString description MEMBER mDescription CONSTANT)
    Q_PROPERTY(QString thumbUrl MEMBER mThumbUrl CONSTANT)


public:
    using Ptr = std::unique_ptr<ExternalView>;

    ExternalView() = default;
    ExternalView(const ExternalView&) = default;
    ExternalView(const QString& uri, const QString& title, const QString& description, const QString& thumbUrl) :
        mUri(uri),
        mTitle(title),
        mDescription(description),
        mThumbUrl(thumbUrl)
    {}

private:
    const QString mUri;
    const QString mTitle;
    const QString mDescription;
    const QString mThumbUrl;
};

}

Q_DECLARE_METATYPE(Skywalker::ExternalView)
