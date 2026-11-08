// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "ozone_subject_view.h"

namespace Skywalker {

class OzoneSubjectListModel : public BaseListModel<OzoneSubjectView>
{
    Q_OBJECT

public:
    enum class Role {
        Subject = Qt::UserRole + 1,
        EndOfList
    };

    using Ptr = std::unique_ptr<OzoneSubjectListModel>;

    OzoneSubjectListModel(const QString& filter, const QString& userDid, QObject* parent = nullptr);

    const QString& getReportsFilter() const { return mReportsFilter; }
    Q_INVOKABLE QDateTime getLatestUpdatedAt() const;
    Q_INVOKABLE bool hasUnread() const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear() override;

    int addSubjects(ATProto::ToolsOzoneInbox::SubjectView::List subjects, const QString& cursor);

    void setCursor(const QString& cursor) override;
    void setEndOfList();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QString mReportsFilter;
};

}