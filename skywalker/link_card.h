// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "external_source.h"
#include "profile.h"
#include "strong_ref.h"
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
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt WRITE setCreatedAt NOTIFY createdAtChanged FINAL)
    Q_PROPERTY(QDateTime updatedAt READ getUpdatedAt WRITE setUpdatedAt NOTIFY updatedAtChanged FINAL)
    Q_PROPERTY(int readingTime READ getReadingTime WRITE setReadingTime NOTIFY readingTimeChanged FINAL)
    Q_PROPERTY(ExternalSource source READ getSource WRITE setSource NOTIFY sourceChanged FINAL);
    Q_PROPERTY(BasicProfileList associatedProfiles READ getAssociatedProfiles WRITE setAssociatedProfiles NOTIFY associatedProfilesChanged FINAL)
    Q_PROPERTY(StrongRef::List associatedRefs READ getAssociatedRefs WRITE setAssociatedRefs NOTIFY associatedRefsChanged FINAL)
    QML_ELEMENT

public:
    explicit LinkCard(QObject* parent = nullptr) : QObject(parent) {}

    const QString& getLink() const { return mLink; }
    const QString& getTitle() const { return mTitle; }
    const QString& getDescription() const { return mDescription; }
    const QString& getThumb() const { return mThumb; }
    QDateTime getCreatedAt() const { return mCreatedAt; }
    QDateTime getUpdatedAt() const { return mUpdatedAt; }
    int getReadingTime() const { return mReadingTime; }
    const ExternalSource& getSource() const { return mSource; }
    const BasicProfileList& getAssociatedProfiles() const { return mAssociatedProfiles; }
    const StrongRef::List& getAssociatedRefs() const { return mAssociatedRefs; }

    void setLink(const QString& link) { mLink = link; emit linkChanged(); }
    void setTitle(const QString& title) { mTitle = title; emit titleChanged(); }
    void setDescription(const QString& description) { mDescription = description; emit descriptionChanged(); }
    void setThumb(const QString& thumb) { mThumb = thumb; emit thumbChanged(); }
    void setCreatedAt(const QDateTime& createdAt) { mCreatedAt = createdAt; emit createdAtChanged(); }
    void setUpdatedAt(const QDateTime& updatedAt) { mUpdatedAt = updatedAt; emit updatedAtChanged(); }
    void setReadingTime(int readingTime) { mReadingTime = readingTime; emit readingTimeChanged(); }
    void setSource(const ExternalSource& source) { mSource = source; emit sourceChanged(); }
    void setAssociatedProfiles(const BasicProfileList& profiles) { mAssociatedProfiles = profiles; emit associatedProfilesChanged(); }
    void setAssociatedRefs(const StrongRef::List& refs) { mAssociatedRefs = refs; emit associatedRefsChanged(); }

    bool isEmpty() const { return mTitle.isEmpty() && mDescription.isEmpty() && mThumb.isEmpty(); }

signals:
    void linkChanged();
    void titleChanged();
    void descriptionChanged();
    void thumbChanged();
    void createdAtChanged();
    void updatedAtChanged();
    void readingTimeChanged();
    void sourceChanged();
    void associatedProfilesChanged();
    void associatedRefsChanged();

private:
    QString mLink;
    QString mTitle;
    QString mDescription;
    QString mThumb;
    QDateTime mCreatedAt;
    QDateTime mUpdatedAt;
    int mReadingTime = 0; // minutes
    ExternalSource mSource;
    BasicProfileList mAssociatedProfiles;
    StrongRef::List mAssociatedRefs;
};

}
