// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QString>
#include <QtQmlIntegration>

namespace Skywalker {

class LinkUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit LinkUtils(QObject* parent = nullptr);

    Q_INVOKABLE void openLink(const QString& link);

signals:
    void webLink(QString uri);
    void authorLink(QString handle);
    void postLink(QString atUri);

private:
    QString isAuthorLink(const QString& link) const;
    QString isPostLink(const QString& link) const;
};

}
