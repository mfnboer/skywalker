// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "muted_words.h"
#include "profile.h"
#include <QVariant>

namespace Skywalker {

class ContentFilterStatItem
{
public:
    using Ptr = std::unique_ptr<ContentFilterStatItem>;

    struct LabelerDid
    {
        QString mDid;
    };

    ContentFilterStatItem(const QString& name, int stat, ContentFilterStatItem* parent = nullptr);
    ContentFilterStatItem(const BasicProfile& profile, int stat, ContentFilterStatItem* parent = nullptr);
    ContentFilterStatItem(const MutedWordEntry& mutedWordEntry, int stat, ContentFilterStatItem* parent = nullptr);
    ContentFilterStatItem(const LabelerDid& labeler, int stat, ContentFilterStatItem* parent = nullptr);

    void clearChildItems() { mChildItems.clear(); };
    void addChild(ContentFilterStatItem::Ptr child);
    ContentFilterStatItem* getChild(int row) const;
    ContentFilterStatItem* getParent() const { return mParentItem; };
    int childCount() const;
    int columnCount() const { return 2; }
    QVariant data(int column) const;
    QEnums::ValueType valueType(int column) const;
    int row() const;

private:
    using Key = std::variant<QString, BasicProfile, MutedWordEntry, LabelerDid>;
    Key mKey;
    int mStat;
    ContentFilterStatItem* mParentItem = nullptr;
    std::vector<ContentFilterStatItem::Ptr> mChildItems;
};

}
