// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "generator_view.h"
#include "list_view.h"
#include "search_feed.h"

namespace Skywalker {

class FavoriteFeedView
{
    Q_GADGET
    Q_PROPERTY(QEnums::FavoriteType type READ getType FINAL)
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(QString avatar READ getAvatar FINAL)
    Q_PROPERTY(QString avatarThumb READ getAvatarThumb FINAL)
    Q_PROPERTY(QEnums::ContentMode contentMode READ getContentMode FINAL)
    Q_PROPERTY(GeneratorView generatorView READ getGeneratorView FINAL)
    Q_PROPERTY(ListView listView READ getListView FINAL)
    Q_PROPERTY(SearchFeed searchFeed READ getSearchFeed FINAL)
    QML_VALUE_TYPE(favoritefeedview)

public:
    FavoriteFeedView() = default;
    explicit FavoriteFeedView(const GeneratorView& generatorView);
    explicit FavoriteFeedView(const ListView& listView);
    explicit FavoriteFeedView(const SearchFeed& searchFeed);

    Q_INVOKABLE bool isNull() const { return mView.index() == 0 && std::get<0>(mView).isNull(); }
    Q_INVOKABLE bool isSame(const FavoriteFeedView& other) const;
    bool operator==(const FavoriteFeedView& other) const { return isSame(other); }

    QEnums::FavoriteType getType() const;
    QString getKey() const; // uri for feeds and lists, name for search feeds
    QString getUri() const;
    QString getName() const;
    QString getAvatar() const;
    QString getAvatarThumb() const;
    QEnums::ContentMode getContentMode() const;
    GeneratorView getGeneratorView() const;
    ListView getListView() const;
    SearchFeed getSearchFeed() const;

private:
    std::variant<GeneratorView, ListView, SearchFeed> mView;
};

}

Q_DECLARE_METATYPE(::Skywalker::FavoriteFeedView)
