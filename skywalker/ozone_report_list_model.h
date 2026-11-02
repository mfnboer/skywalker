// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "ozone_report_view.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class OzoneReportListModel : public QAbstractListModel,
                             public BaseListModel
{
    Q_OBJECT
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)

public:
    enum class Role {
        Report = Qt::UserRole + 1,
        EndOfList
    };

    using Ptr = std::unique_ptr<OzoneReportListModel>;

    OzoneReportListModel(const QString& userDid, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();

    int addReports(ATProto::ToolsOzoneInbox::ReportView::List reports, const QString& cursor);

    const QString& getCursor() const { return mCursor; }
    void setCursor(const QString& cursor);
    bool isEndOfList() const { return mCursor.isEmpty(); }
    void setEndOfList();

    void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

signals:
    void getFeedInProgressChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    using ReportList = std::deque<OzoneReportView>;

    ReportList mReports;
    QString mCursor; // TODO: move to BaseListModel
    const QString& mUserDid; // TODO: move to BaseListModel
    bool mGetFeedInProgress = false;  // TODO: move to BaseListModel
};

}