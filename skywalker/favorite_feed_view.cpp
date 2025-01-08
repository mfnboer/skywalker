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

FavoriteFeedView::FavoriteFeedView(const SearchFeedView& searchFeedView) :
    mView(searchFeedView)
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
    switch (getType())
    {
    case QEnums::FAVORITE_FEED:
        return std::get<GeneratorView>(mView).getDisplayName();
    case QEnums::FAVORITE_LIST:
        return std::get<ListView>(mView).getName();
    case QEnums::FAVORITE_SEARCH:
        return std::get<SearchFeedView>(mView).getSearchQuery();
    }

    Q_ASSERT(false);
    return "unknown";
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

SearchFeedView FavoriteFeedView::getSearchFeedView() const
{
    auto* view = std::get_if<SearchFeedView>(&mView);
    return view ? *view : SearchFeedView{};
}

}
