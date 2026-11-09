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
    mSubjectUriIndexMap.clear();
}

int OzoneSubjectListModel::addSubjects(ATProto::ToolsOzoneInbox::SubjectView::List subjects, const QString& cursor)
{
    qDebug() << "Add subjects:" << subjects.size() << "cursor:" << cursor;
    mCursor = cursor;

    const size_t newRowCount = mList.size() + subjects.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);

    for (const auto& subject : subjects)
    {
        const OzoneSubjectView subjectView(subject);
        mList.emplace_back(subjectView);
        mSubjectUriIndexMap[subjectView.getSubjectUri()] = mList.size() - 1;
    }

    endInsertRows();

    return subjects.size();
}

void OzoneSubjectListModel::updateSubject(const OzoneSubjectView& subject)
{
    const auto it = mSubjectUriIndexMap.find(subject.getSubjectUri());

    if (it == mSubjectUriIndexMap.end())
    {
        qWarning() << "Cannot find subject:" << subject.getSubjectUri();
        return;
    }

    const int index = it->second;

    if (index < 0 || index >= (int)mList.size())
    {
        qWarning() << "Index:" << index << "out of range:" << mList.size() << "subject:" << subject.getSubjectUri();
        return;
    }

    mList[index] = subject;
    const auto modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, { int(Role::Subject) });
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