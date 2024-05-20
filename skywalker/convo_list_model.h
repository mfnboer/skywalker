// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "convo_view.h"
#include <QAbstractListModel>
#include <vector>

namespace Skywalker
{

class ConvoListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Convo = Qt::UserRole + 1,
        EndOfList
    };

    using Ptr = std::unique_ptr<ConvoListModel>;

    explicit ConvoListModel(const QString& userDid, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();
    void addConvos(const ATProto::ChatBskyConvo::ConvoViewList& convos, const QString& cursor);
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    const QString& mUserDid;
    std::vector<ConvoView> mConvos;
    QString mCursor;
};

}
