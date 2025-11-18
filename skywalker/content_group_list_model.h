// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_filter.h"
#include <QAbstractListModel>

namespace Skywalker {

class ContentGroupListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool adultContent READ getAdultContent WRITE setAdultContent NOTIFY adultContentChanged FINAL)
    Q_PROPERTY(bool subscribed READ isSubscribed WRITE setSubscribed NOTIFY subscribedChanged FINAL)
    Q_PROPERTY(bool fixedLabelerEnabled READ isFixedLabelerEnabled WRITE setFixedLabelerEnabled NOTIFY fixedLabelerEnabledChanged FINAL)
    QML_ELEMENT

public:
    enum class Role {
        ContentGroup = Qt::UserRole + 1,
        ContentPrefVisibility,
        IsNewLabel
    };

    using Ptr = std::unique_ptr<ContentGroupListModel>;

    ContentGroupListModel(ContentFilter& contentFilter, const QString& listUri, QObject* parent = nullptr);
    ContentGroupListModel(const QString& labelerDid, ContentFilter& contentFilter, QObject* parent = nullptr);
    void setGlobalContentGroups();
    void setContentGroups(std::vector<ContentGroup> groups);
    void clear();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    const QString& getLabelerDid() const { return mLabelerDid; }
    const QString& getListUri() const { return mListUri; }
    bool hasListPrefs() const { return !mListUri.isEmpty(); }

    bool getAdultContent() const { return mAdultContent; }
    void setAdultContent(bool adultContent);

    bool isSubscribed() const { return mSubscribed; }
    void setSubscribed(bool subscribed);

    bool isFixedLabelerEnabled() const;
    void setFixedLabelerEnabled(bool enabled);

    bool isFixedSubscription() const;

    bool isModified(const ATProto::UserPreferences& userPreferences) const;
    void saveTo(ATProto::UserPreferences& userPreferences) const;
    void saveToContentFilter();

signals:
    void adultContentChanged();
    void subscribedChanged();
    void fixedLabelerEnabledChanged();
    void error(QString msg);

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    void init();

    bool mAdultContent = false;
    ContentFilter& mContentFilter;
    QString mListUri; // filter prefs for "following" or list
    QString mLabelerDid;
    bool mSubscribed = false;
    bool mFixedLabelerEnabled = true;
    std::vector<ContentGroup> mContentGroupList;
    std::unordered_map<int, QEnums::ContentPrefVisibility> mChangedVisibility;
    std::unordered_set<QString> mNewLabelIds;
};

}
