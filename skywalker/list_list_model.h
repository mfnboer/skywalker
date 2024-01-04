// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "list_view.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class ListListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        List = Qt::UserRole + 1,
        ListCreator
    };

    using Type = QEnums::ListPurpose;
    using Ptr = std::unique_ptr<ListListModel>;

    ListListModel(Type type, const QString& atId, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();

    // Returns the number of lists added
    int addLists(ATProto::AppBskyGraph::ListViewList lists, const QString& cursor);

    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    Type getType() const { return mType; }
    const QString& getAtId() const { return mAtId; }

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    using ListList = std::deque<ListView>;

    ListList filterLists(ATProto::AppBskyGraph::ListViewList lists) const;

    Type mType;
    QString mAtId;
    ListList mLists;
    QString mCursor;
};

}
