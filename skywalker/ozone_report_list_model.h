// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "ozone_report_view.h"

namespace Skywalker {

class OzoneReportListModel : public BaseListModel<OzoneReportView>
{
    Q_OBJECT

public:
    enum class Role {
        Report = Qt::UserRole + 1,
        Author,
        EndOfList
    };

    using Ptr = std::unique_ptr<OzoneReportListModel>;

    OzoneReportListModel(const QString& filter, const QString& userDid, QObject* parent = nullptr);

    const QString& getReportsFilter() const { return mReportsFilter; }
    Q_INVOKABLE QDateTime getLatestUpdatedAt() const;
    Q_INVOKABLE bool hasUnread() const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear() override;

    int addReports(ATProto::ToolsOzoneInbox::ReportView::List reports, const QString& cursor);

    void setCursor(const QString& cursor) override;
    void setEndOfList();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    void authorAdded(const QString& did);

    QString mReportsFilter;
};

}