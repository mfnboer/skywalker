// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_report_list_model.h"

namespace Skywalker {

OzoneReportListModel::OzoneReportListModel(const QString& userDid, QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid)
{
}

int OzoneReportListModel::rowCount(const QModelIndex&) const
{
    return mReports.size();
}

QVariant OzoneReportListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mReports.size())
        return {};

    const auto& report = mReports[index.row()];

    switch (Role(role))
    {
    case Role::Report:
        return QVariant::fromValue(report);
    case Role::EndOfList:
        return isEndOfList() && index.row() == (int)mReports.size() - 1;
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void OzoneReportListModel::clear()
{
    qDebug() << "Clear reports";

    if (!mReports.empty())
    {
        beginRemoveRows({}, 0, mReports.size() - 1);
        mReports.clear();
        endRemoveRows();
    }
}

int OzoneReportListModel::addReports(ATProto::ToolsOzoneInbox::ReportView::List reports, const QString& cursor)
{
    qDebug() << "Add reports:" << reports.size() << "cursor:" << cursor;
    mCursor = cursor;

    const size_t newRowCount = mReports.size() + reports.size();

    beginInsertRows({}, mReports.size(), newRowCount - 1);

    for (const auto& report : reports)
        mReports.emplace_back(report);

    endInsertRows();

    return reports.size();
}

void OzoneReportListModel::setEndOfList()
{
    if (isEndOfList() && !mReports.empty())
    {
        const auto index = createIndex(mReports.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfList) });
    }
}

void OzoneReportListModel::setGetFeedInProgress(bool inProgress)
{
    if (inProgress != mGetFeedInProgress) {
        mGetFeedInProgress = inProgress;
        emit getFeedInProgressChanged();
    }
}

QHash<int, QByteArray> OzoneReportListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Report), "report" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

}