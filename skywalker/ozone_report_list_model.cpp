// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_report_list_model.h"
#include "author_cache.h"

namespace Skywalker {

OzoneReportListModel::OzoneReportListModel(const QString& filter, const QString& userDid, QObject* parent) :
    BaseType(userDid, parent),
    mReportsFilter(filter)
{
    connect(&AuthorCache::instance(), &AuthorCache::profileAdded, this,
        [this](const QString& did) { authorAdded(did); },
        Qt::QueuedConnection);
}

QDateTime OzoneReportListModel::getLatestUpdatedAt() const
{
    if (mList.empty())
        return {};

    return mList.front().getUpdatedAt();
}

bool OzoneReportListModel::hasUnread() const
{
    if (mList.empty())
        return false;

    return !mList.front().isRead();
}

QVariant OzoneReportListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& report = mList[index.row()];

    switch (Role(role))
    {
    case Role::Report:
        return QVariant::fromValue(report);
    case Role::Author:
    {
        const QString did = report.getSubjectDid();

        if (did.isEmpty())
            return {};

        const auto* author = AuthorCache::instance().get(did);

        if (!author)
        {
            QTimer::singleShot(0, this, [did]{ AuthorCache::instance().putProfile(did); });
            return {};
        }

        return QVariant::fromValue(*author);
    }
    case Role::EndOfList:
        return isEndOfList() && index.row() == (int)mList.size() - 1;
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void OzoneReportListModel::clear()
{
    qDebug() << "Clear reports";
    BaseType::clear();
}

int OzoneReportListModel::addReports(ATProto::ToolsOzoneInbox::ReportView::List reports, const QString& cursor)
{
    qDebug() << "Add reports:" << reports.size() << "cursor:" << cursor;
    mCursor = cursor;

    const size_t newRowCount = mList.size() + reports.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);

    for (const auto& report : reports)
        mList.emplace_back(report);

    endInsertRows();

    return reports.size();
}

void OzoneReportListModel::setCursor(const QString& cursor)
{
    BaseType::setCursor(cursor);
    setEndOfList();
}

void OzoneReportListModel::setEndOfList()
{
    if (isEndOfList() && !mList.empty())
    {
        const auto index = createIndex(mList.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfList) });
    }
}

QHash<int, QByteArray> OzoneReportListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Report), "report" },
        { int(Role::Author), "author" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

void OzoneReportListModel::authorAdded(const QString& did)
{
    for (int i = 0; i < (int)mList.size(); ++i)
    {
        const auto& report = mList[i];

        if (report.getSubjectDid() == did)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::Author) });
    }
}

}