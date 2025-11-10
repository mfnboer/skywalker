// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "content_filter_stat_item.h"

namespace Skywalker {

ContentFilterStatItem::ContentFilterStatItem(const QString& name, int stat, ContentFilterStatItem* parent) :
    mKey(name),
    mStat(stat),
    mParentItem(parent)
{
}

ContentFilterStatItem::ContentFilterStatItem(const BasicProfile& profile, int stat, ContentFilterStatItem* parent) :
    mKey(profile),
    mStat(stat),
    mParentItem(parent)
{
}

ContentFilterStatItem::ContentFilterStatItem(const MutedWordEntry& mutedWordEntry, int stat, ContentFilterStatItem* parent) :
    mKey(mutedWordEntry),
    mStat(stat),
    mParentItem(parent)
{
}

ContentFilterStatItem::ContentFilterStatItem(const LabelerDid& labeler, int stat, ContentFilterStatItem* parent) :
    mKey(labeler),
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
        if (std::holds_alternative<LabelerDid>(mKey))
            return std::get<LabelerDid>(mKey).mDid;

        return std::visit([](auto&& key){ return QVariant::fromValue(key); }, mKey);
    case 1:
        return mStat;
    default:
        return {};
    }
}

QEnums::ValueType ContentFilterStatItem::valueType(int column) const
{
    switch (column)
    {
    case 0:
        if (std::holds_alternative<QString>(mKey))
            return QEnums::VALUE_TYPE_STRING;
        if (std::holds_alternative<BasicProfile>(mKey))
            return QEnums::VALUE_TYPE_BASIC_PROFILE;
        if (std::holds_alternative<MutedWordEntry>(mKey))
            return QEnums::VALUE_TYPE_MUTED_WORD_ENTRY;
        if (std::holds_alternative<LabelerDid>(mKey))
            return QEnums::VALUE_TYPE_LABELER_DID;

        qWarning() << "Unknown key type";
        return QEnums::VALUE_TYPE_STRING;
    case 1:
        return QEnums::VALUE_TYPE_INT;
    default:
        qWarning() << "Unknown column:" << column;
        return QEnums::VALUE_TYPE_STRING;
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
