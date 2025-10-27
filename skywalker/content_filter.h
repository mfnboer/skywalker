// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "content_group.h"
#include <tuple>

namespace Skywalker {

class UserSettings;

class IContentFilter
{
public:
    virtual ~IContentFilter() = default;
    virtual std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::Label::List& labels,
                                                                                   std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const = 0;
    virtual std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ContentLabelList& contentLabels,
                                                                                   std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const = 0;
};

class ContentFilter : public QObject, public IContentFilter
{
    Q_OBJECT

public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>;
    using GlobalContentGroupMap = std::unordered_map<QString, const ContentGroup*>;

    // System labels have a hard coded setting.
    static const std::vector<ContentGroup> SYSTEM_CONTENT_GROUP_LIST;

    // User labels have a default setting that can be changed by the user.
    static const std::vector<ContentGroup> USER_CONTENT_GROUP_LIST;

    // System and user labels make up the global labels.
    static const GlobalContentGroupMap& getGlobalContentGroups();
    static const ContentGroup* getGlobalContentGroup(const QString& labelId);
    static bool isGlobalLabel(const QString& labelId);

    // This function removes neg-labels, i.e. if X and not-X are labels, then X is not in the result.
    static ContentLabelList getContentLabels(const LabelList& labels);
    static void addContentLabels(ContentLabelList& contentLabels, const LabelList& labels);

    explicit ContentFilter(const QString& userDid, const ATProto::UserPreferences& userPreferences, UserSettings* userSettings, QObject* parent = nullptr);

    void clear();

    // Returns a global content group if the labelId is a global label
    const ContentGroup* getContentGroup(const QString& did, const QString& labelId) const;

    Q_INVOKABLE bool getAdultContent() const { return mUserPreferences.getAdultContent(); }

    QEnums::ContentPrefVisibility getGroupPrefVisibility(const ContentGroup& group) const;
    QEnums::ContentVisibility getGroupVisibility(const ContentGroup& group, std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const;
    QEnums::ContentVisibility getVisibility(const ContentLabel& label, std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const;
    Q_INVOKABLE bool mustShowBadge(const ContentLabel& label) const;
    QString getGroupWarning(const ContentGroup& group) const;
    QString getWarning(const ContentLabel& label) const;

    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::Label::List& labels,
                                                                           std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const override;
    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ContentLabelList& contentLabels,
                                                                           std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const override;

    bool isSubscribedToLabeler(const QString& did) const;
    bool isFixedLabelerEnabled(const QString& did) const;
    void enableFixedLabeler(const QString& did, bool enabled);

    std::unordered_set<QString> getSubscribedLabelerDids(bool includeDisabledFixedLabelers = false) const;
    std::vector<QString> getSubscribedLabelerDidsOrdered() const;
    size_t numLabelers() const;
    QStringList getLabelIds(const QString& labelerDid) const;

    void addContentGroupMap(const QString& did, const ContentGroupMap& contentGroupMap);
    void addContentGroups(const QString& did, const std::vector<ContentGroup>& contentGroups);
    void removeContentGroups(const QString& did);

    Q_INVOKABLE void saveLabelIdsToSettings(const QString& labelerDid) const;
    void removeLabelIdsFromSettings(const QString &labelerDid) const;
    std::unordered_set<QString> getNewLabelIds(const QString& labelerDid) const;
    Q_INVOKABLE bool hasNewLabels(const QString& labelerDid) const;
    std::unordered_set<QString> getLabelerDidsWithNewLabels() const;

    static bool isFixedLabelerSubscription(const QString& did);

signals:
    void contentGroupsChanged();
    void subscribedLabelersChanged();

private:
    static GlobalContentGroupMap CONTENT_GROUPS;
    static void initContentGroups();

    const QString& mUserDid;
    const ATProto::UserPreferences& mUserPreferences;
    UserSettings* mUserSettings;
    std::unordered_map<QString, ContentGroupMap> mLabelerGroupMap; // labeler DID -> group map
};

class ContentFilterShowAll : public IContentFilter
{
public:
    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::Label::List&, std::optional<QEnums::ContentVisibility> = {}) const override
    {
        return {QEnums::CONTENT_VISIBILITY_SHOW, ""};
    }

    virtual std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ContentLabelList&, std::optional<QEnums::ContentVisibility> = {}) const override
    {
        return {QEnums::CONTENT_VISIBILITY_SHOW, ""};
    }
};

}
