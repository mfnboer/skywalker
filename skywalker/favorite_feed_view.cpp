// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "favorite_feed_view.h"

namespace Skywalker {

FavoriteFeedView::FavoriteFeedView(const GeneratorView& generatorView) :
    mView(generatorView)
{
}

FavoriteFeedView::FavoriteFeedView(const ListView& listView) :
    mView(listView)
{
}

FavoriteFeedView::FavoriteFeedView(const SearchFeed& searchFeed) :
    mView(searchFeed)
{
}

QEnums::FavoriteType FavoriteFeedView::getType() const
{
    return std::visit([](auto&& view){ return view.getFavoriteType(); }, mView);
}

QString FavoriteFeedView::getUri() const
{
    switch (getType())
    {
    case QEnums::FAVORITE_FEED:
        return std::get<GeneratorView>(mView).getUri();
    case QEnums::FAVORITE_LIST:
        return std::get<ListView>(mView).getUri();
    case QEnums::FAVORITE_SEARCH:
        return "";
    }

    Q_ASSERT(false);
    return "";
}

QString FavoriteFeedView::getName() const
{
    return std::visit([](auto&& view){ return view.getName(); }, mView);
}

QString FavoriteFeedView::getAvatar() const
{
    switch (getType())
    {
    case QEnums::FAVORITE_FEED:
        return std::get<GeneratorView>(mView).getAvatar();
    case QEnums::FAVORITE_LIST:
        return std::get<ListView>(mView).getAvatar();
    case QEnums::FAVORITE_SEARCH:
        return "";
    }

    Q_ASSERT(false);
    return "";
}

QString FavoriteFeedView::getAvatarThumb() const
{
    return ATProto::createAvatarThumbUrl(getAvatar());
}

GeneratorView FavoriteFeedView::getGeneratorView() const
{
    auto* view = std::get_if<GeneratorView>(&mView);
    return view ? *view : GeneratorView{};
}

ListView FavoriteFeedView::getListView() const
{
    auto* view = std::get_if<ListView>(&mView);
    return view ? *view : ListView{};
}

SearchFeed FavoriteFeedView::getSearchFeed() const
{
    auto* view = std::get_if<SearchFeed>(&mView);
    return view ? *view : SearchFeed{};
}

bool FavoriteFeedView::isSame(const FavoriteFeedView& other) const
{
    if (getType() != other.getType())
        return false;

    switch (getType())
    {
    case QEnums::FAVORITE_FEED:
    case QEnums::FAVORITE_LIST:
        return getName() == other.getName() && getUri() == other.getUri() && getAvatar() == other.getAvatar();
    case QEnums::FAVORITE_SEARCH:
        return getSearchFeed().equals(other.getSearchFeed());
    }

    return false;
}

}
