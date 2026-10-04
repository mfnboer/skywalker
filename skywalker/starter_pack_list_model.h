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
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)

public:
    enum class Role {
        StarterPack = Qt::UserRole
    };

    using Ptr = std::unique_ptr<StarterPackListModel>;

    explicit StarterPackListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();
    void addStarterPacks(ATProto::AppBskyGraph::StarterPackViewBasic::List starterPacks, const QString& cursor);
    void addStarterPacks(ATProto::AppBskyGraph::StarterPackView::List starterPacks, const QString& cursor);

    Q_INVOKABLE void prependList(const StarterPackViewBasic& starterPack);
    Q_INVOKABLE StarterPackViewBasic updateEntry(int index, const QString& cid, const QString& name,
                                                 const QString& description, const NamedLink::List& embeddedLinks);
    Q_INVOKABLE void deleteEntry(int index);
    Q_INVOKABLE StarterPackViewBasic getEntry(int index) const;

    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

signals:
    void getFeedInProgressChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    template<typename T>
    void _addStarterPacks(const std::vector<T>& starterPacks, const QString& cursor);

    using StarterPackList = std::deque<StarterPackViewBasic>;
    StarterPackList mStarterPacks;
    QString mCursor;
    bool mGetFeedInProgress = false;
};

}
