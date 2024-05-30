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
    void updateConvo(const ATProto::ChatBskyConvo::ConvoView& convo);
    const ConvoView* getConvo(const QString& convoId) const;
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }
    QString getLastRev() const;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    void changeData(const QList<int>& roles, int begin = 0, int end = -1);

    const QString& mUserDid;
    std::vector<ConvoView> mConvos;
    std::unordered_map<QString, int> mConvoIdIndexMap;
    QString mCursor;
};

}
