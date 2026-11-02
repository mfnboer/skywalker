// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_base_list_model.h"
#include <QList>
#include <deque>

namespace Skywalker {

template<class EntityType>
class BaseListModel : public AbstractBaseListModel
{
public:
    using BaseType = BaseListModel<EntityType>;

    explicit BaseListModel(const QString& userDid, QObject* parent = nullptr) : AbstractBaseListModel(userDid, parent) {}

    virtual void clear()
    {
        if (!mList.empty())
        {
            beginRemoveRows({}, 0, mList.size() - 1);
            mList.clear();
            endRemoveRows();
        }

        mCursor.clear();
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return mList.size();
    }

    virtual void refreshAllData() { changeData({}); }

    const QString& getCursor() const { return mCursor; }
    virtual void setCursor(const QString& cursor) { mCursor = cursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

protected:
    using EntityList = std::deque<EntityType>;

    virtual void changeData(const QList<int>& roles) = 0;

    EntityList mList;
    QString mCursor;
};

}
