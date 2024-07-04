// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "starter_pack.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class StarterPackListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        StarterPack = Qt::UserRole
    };

    using Ptr = std::unique_ptr<StarterPackListModel>;

    explicit StarterPackListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();
    void addStarterPacks(ATProto::AppBskyGraph::StarterPackViewBasicList starterPacks, const QString& cursor);
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    using StarterPackList = std::deque<StarterPackViewBasic>;
    StarterPackList mStarterPacks;
    QString mCursor;
};

}
