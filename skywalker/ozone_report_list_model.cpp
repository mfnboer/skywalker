// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_report_list_model.h"

namespace Skywalker {

OzoneReportListModel::OzoneReportListModel(const QString& userDid, QObject* parent) :
    BaseType(userDid, parent)
{
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
    BaseListModel<OzoneReportView>::setCursor(cursor);
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
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

}