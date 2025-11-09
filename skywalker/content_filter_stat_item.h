// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QVariant>

namespace Skywalker {

class ContentFilterStatItem
{
public:
    using Ptr = std::unique_ptr<ContentFilterStatItem>;

    ContentFilterStatItem(const QString& name, int stat, ContentFilterStatItem* parent = nullptr);

    void clearChildItems() { mChildItems.clear(); };
    void addChild(ContentFilterStatItem::Ptr child);
    ContentFilterStatItem* getChild(int row) const;
    ContentFilterStatItem* getParent() const { return mParentItem; };
    int childCount() const;
    int columnCount() const { return 2; }
    QVariant data(int column) const;
    int row() const;

private:
    QString mName;
    int mStat;
    ContentFilterStatItem* mParentItem = nullptr;
    std::vector<ContentFilterStatItem::Ptr> mChildItems;
};

}
