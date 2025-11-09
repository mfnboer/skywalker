// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "content_filter_stat_item.h"

namespace Skywalker {

ContentFilterStatItem::ContentFilterStatItem(const QString& name, int stat, ContentFilterStatItem* parent) :
    mName(name),
    mStat(stat),
    mParentItem(parent)
{
}

void ContentFilterStatItem::addChild(ContentFilterStatItem::Ptr child)
{
    mChildItems.push_back(std::move(child));
}

ContentFilterStatItem* ContentFilterStatItem::getChild(int row) const
{
    if (row < 0 || row >= (int)mChildItems.size())
        return nullptr;

    return mChildItems.at(row).get();
}

int ContentFilterStatItem::childCount() const
{
    return (int)mChildItems.size();
}

QVariant ContentFilterStatItem::data(int column) const
{
    switch (column)
    {
    case 0:
        return mName;
    case 1:
        return mStat;
    default:
        return {};
    }
}

int ContentFilterStatItem::row() const
{
    if (!mParentItem)
        return 0;

    const auto it = std::find_if(mParentItem->mChildItems.cbegin(), mParentItem->mChildItems.cend(),
                                 [this](const auto& item){ return item.get() == this; });

    if (it != mParentItem->mChildItems.end())
        return it - mParentItem->mChildItems.begin();

    Q_ASSERT(false);
    return -1;
}

}
