// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "language_utils.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class SearchFeed
{
    Q_GADGET
    Q_PROPERTY(QString name READ getName() FINAL)
    Q_PROPERTY(QString searchQuery READ getSearchQuery FINAL)
    Q_PROPERTY(QString authorHandle READ getAuthorHandle FINAL)
    Q_PROPERTY(QString mentionHandle READ getMentionHandle FINAL)
    Q_PROPERTY(QDateTime since READ getSince FINAL)
    Q_PROPERTY(QDateTime until READ getUntil FINAL)
    Q_PROPERTY(QString language READ getLanguage FINAL)
    Q_PROPERTY(LanguageList languageList READ getLanguageList FINAL)
    QML_VALUE_TYPE(searchfeed)

public:
    using List = QList<SearchFeed>;

    SearchFeed() = default;
    SearchFeed(const QString& searchQuery, const QString& authorHandle, const QString& mentionsHandle,
                   QDateTime since, QDateTime until, const QString& language);

    QEnums::FavoriteType getFavoriteType() const { return QEnums::FAVORITE_SEARCH; }

    Q_INVOKABLE bool isNull() const { return mSearchQuery.isEmpty(); }
    Q_INVOKABLE bool isHashtag() const;
    Q_INVOKABLE bool equals(const SearchFeed& other) const;

    const QString& getName() const { return mSearchQuery; }
    const QString& getSearchQuery() const { return mSearchQuery; }
    const QString& getAuthorHandle() const { return mAuthorHandle; }
    const QString& getMentionHandle() const { return mMentionHandle; }
    QDateTime getSince() const { return mSince; }
    QDateTime getUntil() const { return mUntil; }
    const QString& getLanguage() const { return mLanguage; }
    LanguageList getLanguageList() const;

    QJsonObject toJson() const;
    static SearchFeed fromJson(const QJsonObject& json);

private:
    QString mSearchQuery;
    QString mAuthorHandle; // "", "me" or real handle
    QString mMentionHandle;
    QDateTime mSince;
    QDateTime mUntil;
    QString mLanguage;
};

}
