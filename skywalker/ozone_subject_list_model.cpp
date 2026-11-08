// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_subject_list_model.h"

namespace Skywalker {

OzoneSubjectListModel::OzoneSubjectListModel(const QString& filter, const QString& userDid, QObject* parent) :
    BaseType(userDid, parent),
    mReportsFilter(filter)
{
}

QDateTime OzoneSubjectListModel::getLatestUpdatedAt() const
{
    if (mList.empty())
        return {};

    return mList.front().getUpdatedAt();
}

bool OzoneSubjectListModel::hasUnread() const
{
    if (mList.empty())
        return false;

    return !mList.front().isRead();
}

QVariant OzoneSubjectListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& subject = mList[index.row()];

    switch (Role(role))
    {
    case Role::Subject:
        return QVariant::fromValue(subject);
    case Role::EndOfList:
        return isEndOfList() && index.row() == (int)mList.size() - 1;
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void OzoneSubjectListModel::clear()
{
    qDebug() << "Clear subjects";
    BaseType::clear();
}

int OzoneSubjectListModel::addSubjects(ATProto::ToolsOzoneInbox::SubjectView::List subjects, const QString& cursor)
{
    qDebug() << "Add subjects:" << subjects.size() << "cursor:" << cursor;
    mCursor = cursor;

    const size_t newRowCount = mList.size() + subjects.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);

    for (const auto& subject : subjects)
        mList.emplace_back(subject);

    endInsertRows();

    return subjects.size();
}

void OzoneSubjectListModel::setCursor(const QString& cursor)
{
    BaseType::setCursor(cursor);
    setEndOfList();
}

void OzoneSubjectListModel::setEndOfList()
{
    if (isEndOfList() && !mList.empty())
    {
        const auto index = createIndex(mList.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfList) });
    }
}

QHash<int, QByteArray> OzoneSubjectListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Subject), "subject" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

}