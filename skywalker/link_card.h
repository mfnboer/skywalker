// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class LinkCard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString link READ getLink WRITE setLink NOTIFY linkChanged FINAL);
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged FINAL);
    Q_PROPERTY(QString description READ getDescription WRITE setDescription NOTIFY descriptionChanged FINAL);
    Q_PROPERTY(QString thumb READ getThumb WRITE setThumb NOTIFY thumbChanged FINAL);
    QML_ELEMENT

public:
    explicit LinkCard(QObject* parent = nullptr) : QObject(parent) {}

    const QString& getLink() const { return mLink; }
    const QString& getTitle() const { return mTitle; }
    const QString& getDescription() const { return mDescription; }
    const QString& getThumb() const { return mThumb; }

    void setLink(const QString& link) { mLink = link; emit linkChanged(); }
    void setTitle(const QString& title) { mTitle = title; emit titleChanged(); }
    void setDescription(const QString& description) { mDescription = description; emit descriptionChanged(); }
    void setThumb(const QString& thumb) { mThumb = thumb; emit thumbChanged(); }

    bool isEmpty() const { return mTitle.isEmpty() && mDescription.isEmpty() && mThumb.isEmpty(); }

signals:
    void linkChanged();
    void titleChanged();
    void descriptionChanged();
    void thumbChanged();

private:
    QString mLink;
    QString mTitle;
    QString mDescription;
    QString mThumb;
};

}
