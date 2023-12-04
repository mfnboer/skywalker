// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class TenorCategory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER mName CONSTANT FINAL)
    Q_PROPERTY(QString gifUrl MEMBER mGifUrl CONSTANT FINAL)
    Q_PROPERTY(QString searchTerm MEMBER mSearchTerm CONSTANT FINAL)
    QML_ELEMENT

public:
    TenorCategory(const QString& name, const QString& gifUrl, const QString& searchTerm,
                  QObject* parent = nullptr) :
        QObject(parent),
        mName(name),
        mGifUrl(gifUrl),
        mSearchTerm(searchTerm)
    {}

private:
    QString mName;
    QString mGifUrl;
    QString mSearchTerm;
};

using TenorCategoryList = QList<TenorCategory>;

}
