// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "language_utils.h"
#include "search_options.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class SearchFeed
{
    Q_GADGET
    Q_PROPERTY(QString key READ getKey FINAL)
    Q_PROPERTY(QString name READ getName() FINAL)
    Q_PROPERTY(QString searchQuery READ getSearchQuery FINAL)
    Q_PROPERTY(SearchOptions searchOptions READ getSearchOptions FINAL)
    Q_PROPERTY(LanguageList languageList READ getLanguageList FINAL)
    QML_VALUE_TYPE(searchfeed)

public:
    using List = QList<SearchFeed>;

    SearchFeed() = default;
    SearchFeed(const QString& searchQuery, const SearchOptions& searchOptions);

    QEnums::FavoriteType getFavoriteType() const { return QEnums::FAVORITE_SEARCH; }

    Q_INVOKABLE bool isNull() const { return mSearchQuery.isEmpty(); }
    Q_INVOKABLE bool isHashtag() const;
    Q_INVOKABLE bool isCashtag() const;
    Q_INVOKABLE bool equals(const SearchFeed& other) const;

    const QString getKey() const;
    const QString& getName() const { return mSearchQuery; }
    const QString& getSearchQuery() const { return mSearchQuery; }
    const SearchOptions& getSearchOptions() const { return mSearchOptions; }
    LanguageList getLanguageList() const;

    QJsonObject toJson() const;
    static SearchFeed fromJson(const QJsonObject& json);

private:
    QString mSearchQuery;
    SearchOptions mSearchOptions;
};

}
