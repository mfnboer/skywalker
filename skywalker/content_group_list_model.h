// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_filter.h"
#include <QAbstractListModel>

namespace Skywalker {

class ContentGroupListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool adultContent READ getAdultContent() WRITE setAdultContent() NOTIFY adultContentChanged() FINAL)

public:
    enum class Role {
        ContentGroup = Qt::UserRole + 1,
        ContentPrefVisibility
    };

    using Ptr = std::unique_ptr<ContentGroupListModel>;

    explicit ContentGroupListModel(const ContentFilter& contentFilter, QObject* parent = nullptr);
    void setGlobalContentGroups();
    void setContentGroups(std::vector<ContentGroup> groups);
    void clear();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool getAdultContent() const { return mAdultContent; }
    void setAdultContent(bool adultContent);

    bool isModified(const ATProto::UserPreferences& userPreferences) const;
    void saveTo(ATProto::UserPreferences& userPreferences) const;

signals:
    void adultContentChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    bool mAdultContent = false;
    const ContentFilter& mContentFilter;
    std::vector<ContentGroup> mContentGroupList;
    std::unordered_map<int, QEnums::ContentPrefVisibility> mChangedVisibility;
};

}
