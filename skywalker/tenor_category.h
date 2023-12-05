// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class TenorCategory
{
    Q_GADGET
    Q_PROPERTY(QString gifUrl MEMBER mGifUrl CONSTANT FINAL)
    Q_PROPERTY(QString searchTerm MEMBER mSearchTerm CONSTANT FINAL)
    QML_VALUE_TYPE(tenorcategory)

public:
    TenorCategory() = default;

    TenorCategory(const QString& gifUrl, const QString& searchTerm) :
        mGifUrl(gifUrl),
        mSearchTerm(searchTerm)
    {}

private:
    QString mGifUrl;
    QString mSearchTerm;
};

using TenorCategoryList = QList<TenorCategory>;

}

Q_DECLARE_METATYPE(Skywalker::TenorCategory)
