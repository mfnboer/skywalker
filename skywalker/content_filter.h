// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "content_group.h"
#include <tuple>

namespace Skywalker {

class IProfileStore;
class ListStore;
class UserSettings;

class IContentFilter
{
public:
    virtual ~IContentFilter() = default;
    virtual std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(
        const QString& authorDid,
        const ATProto::ComATProtoLabel::Label::List& labels,
        std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const = 0;
    virtual std::tuple<QEnums::ContentVisibility, QString, int> getVisibilityAndWarning(
        const QString& authorDid,
        const ContentLabelList& contentLabels,
        std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const = 0;
    virtual const ContentGroup* getContentGroup(const QString& did, const QString& labelId) const = 0;
};

class ContentFilter : public QObject, public IContentFilter
{
    Q_OBJECT
    Q_PROPERTY(bool hasFollowingPrefs READ hasFollowingPrefs NOTIFY hasFollowingPrefsChanged FINAL)
    QML_ELEMENT

public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>;
    using GlobalContentGroupMap = std::unordered_map<QString, const ContentGroup*>;

    static const QString BLUESKY_MODERATOR_DID;

    // System labels have a hard coded setting.
    static const std::vector<ContentGroup> SYSTEM_CONTENT_GROUP_LIST;

    // We allow overriding the settins for some system labels
    static const std::unordered_set<QString> OVERRIDABLE_SYSTEM_LABELS_IDS;

    // User labels have a default setting that can be changed by the user.
    static const std::vector<ContentGroup> USER_CONTENT_GROUP_LIST;

    // System and user labels make up the global labels.
    static const GlobalContentGroupMap& getGlobalContentGroups();
    static const ContentGroup* getGlobalContentGroup(const QString& labelId);
    static bool isGlobalLabel(const QString& labelId);
    static bool isSystemLabelId(const QString& labelId);
    static bool isOverridableSytemLabelId(const QString& labelId);

    // This function removes neg-labels, i.e. if X and not-X are labels, then X is not in the result.
    static ContentLabelList getContentLabels(const LabelList& labels);
    static void addContentLabels(ContentLabelList& contentLabels, const LabelList& labels);

    explicit ContentFilter(const QString& userDid,
                           const IProfileStore& following,
                           const ListStore& policies,
                           const ATProto::UserPreferences& userPreferences,
                           UserSettings* userSettings, QObject* parent = nullptr);

    void clear();
    void initListPrefs();

    // Returns a global content group if the labelId is a global label
    const ContentGroup* getContentGroup(const QString& did, const QString& labelId) const override;

    Q_INVOKABLE bool getAdultContent() const { return mUserPreferences.getAdultContent(); }

    Q_INVOKABLE QEnums::ContentPrefVisibility getGroupPrefVisibility(
        const ContentGroup& group, const QString& listUri = {}) const;
    QEnums::ContentVisibility getGroupVisibility(
        const QString& authorDid,
        const ContentGroup& group,
        std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const;
    QEnums::ContentVisibility getVisibility(
        const QString& authorDid,
        const ContentLabel& label,
        std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const;
    Q_INVOKABLE bool mustShowBadge(const QString& authorDid, const ContentLabel& label) const;
    QString getGroupWarning(const ContentGroup& group) const;
    QString getWarning(const ContentLabel& label) const;

    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(
        const QString& authorDid,
        const ATProto::ComATProtoLabel::Label::List& labels,
        std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const override;
    std::tuple<QEnums::ContentVisibility, QString, int> getVisibilityAndWarning(
        const QString& authorDid,
        const ContentLabelList& contentLabels,
        std::optional<QEnums::ContentVisibility> adultOverrideVisibility = {}) const override;

    bool isSubscribedToLabeler(const QString& did) const;
    bool isFixedLabelerEnabled(const QString& did) const;
    void enableFixedLabeler(const QString& did, bool enabled);

    std::unordered_set<QString> getSubscribedLabelerDids(bool includeDisabledFixedLabelers = false) const;
    std::vector<QString> getSubscribedLabelerDidsOrdered() const;
    size_t numLabelers() const;

    void addContentGroupMap(const QString& did, const ContentGroupMap& contentGroupMap);
    void addContentGroups(const QString& did, const std::vector<ContentGroup>& contentGroups);
    void removeContentGroups(const QString& did);

    Q_INVOKABLE void saveLabelIdsToSettings(const QString& labelerDid) const;
    void removeLabelIdsFromSettings(const QString &labelerDid) const;
    std::unordered_set<QString> getNewLabelIds(const QString& labelerDid) const;
    Q_INVOKABLE bool hasNewLabels(const QString& labelerDid) const;
    std::unordered_set<QString> getLabelerDidsWithNewLabels() const;

    bool hasFollowingPrefs() const;
    void clearFollowingPrefs();
    Q_INVOKABLE void createFollowingPrefs();

    Q_INVOKABLE bool hasListPref(const QString& listUri, const QString& labelerDid) const;
    Q_INVOKABLE void createListPref(const QString& listUri);

    void setListPref(const QString& listUri, const QString& labelerDid, const QString& labelId, QEnums::ContentPrefVisibility pref);
    void removeListPref(const QString& listUri, const QString& labelerDid, const QString& labelId);

    Q_INVOKABLE QString getListName(const QString& listUri) const;

    static bool isFixedLabelerSubscription(const QString& did);

signals:
    void contentGroupsChanged(const QString& listUri);
    void subscribedLabelersChanged();
    void hasFollowingPrefsChanged();
    void listPrefsChanged(const QString& listUri, const QString& labelerDid);

private:
    static GlobalContentGroupMap CONTENT_GROUPS;
    static void initContentGroups();
    void removeListPrefs(const QString& listUri);
    QStringList getLabelIds(const QString& labelerDid) const;
    ATProto::UserPreferences::LabelVisibility getVisibilityDefaultPrefs(const ContentGroup& group) const;
    ATProto::UserPreferences::LabelVisibility getVisibilityAuthorPrefs(const QString& authorDid, const ContentGroup& group) const;
    ATProto::UserPreferences::LabelVisibility getLabelVisibility(
        const ATProto::UserPreferences::ContentLabelPrefs& labelPrefs, const ContentGroup& group) const;
    const ATProto::UserPreferences::ContentLabelPrefs* getContentLabelPrefs(const QString& listUri) const;
    ATProto::UserPreferences::ContentLabelPrefs* getContentLabelPrefs(const QString& listUri);

    const QString& mUserDid;
    const IProfileStore& mFollowing;
    const ListStore& mListsWithPolicies;
    const ATProto::UserPreferences& mUserPreferences;
    UserSettings* mUserSettings;
    std::unordered_map<QString, ContentGroupMap> mLabelerGroupMap; // labeler DID -> group map

    ATProto::UserPreferences::ContentLabelPrefs mFollowingPrefs; // labeler DID -> label visibility
    std::unordered_map<QString, ATProto::UserPreferences::ContentLabelPrefs> mListPrefs; // list uri -> prefs
};

class ContentFilterShowAll : public IContentFilter
{
public:
    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(
        const QString&,
        const ATProto::ComATProtoLabel::Label::List&,
        std::optional<QEnums::ContentVisibility> = {}) const override
    {
        return {QEnums::CONTENT_VISIBILITY_SHOW, ""};
    }

    virtual std::tuple<QEnums::ContentVisibility, QString, int> getVisibilityAndWarning(
        const QString&,
        const ContentLabelList&,
        std::optional<QEnums::ContentVisibility> = {}) const override
    {
        return {QEnums::CONTENT_VISIBILITY_SHOW, "", -1};
    }

    const ContentGroup* getContentGroup(const QString&, const QString&) const override
    {
        return nullptr;
    }
};

}
