// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "content_group.h"
#include <tuple>

namespace Skywalker {

class IContentFilter
{
public:
    virtual ~IContentFilter() = default;
    virtual std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::LabelList& labels) const = 0;
};

class ContentFilter : public QObject, public IContentFilter
{
    Q_OBJECT

public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::Ptr>;
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

    explicit ContentFilter(const ATProto::UserPreferences& userPreferences, QObject* parent = nullptr);

    // Returns a global content group if the labelId is a global label
    const ContentGroup* getContentGroup(const QString& did, const QString& labelId) const;

    QEnums::ContentVisibility getGroupVisibility(const ContentGroup& group) const;
    QEnums::ContentVisibility getVisibility(const ContentLabel& label) const;
    QString getGroupWarning(const ContentGroup& group) const;
    QString getWarning(const ContentLabel& label) const;

    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::LabelList& labels) const override;
    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ContentLabelList& contentLabels) const;

    bool isSubscribedToLabeler(const QString& did) const;
    std::unordered_set<QString> getSubscribedLabelerDids() const;
    size_t numLabelers() const;

    void addContentGroupMap(const QString& did, const ContentGroupMap& contentGroupMap);
    void addContentGroups(const QString& did, const std::vector<ContentGroup>& contentGroups);
    void removeContentGroups(const QString& did);

    static bool isFixedLabelerSubscription(const QString& did);

signals:
    void contentGroupsChanged();

private:
    static GlobalContentGroupMap CONTENT_GROUPS;
    static void initContentGroups();
    static void initLabelGroupMap();

    static std::unordered_map<QString, QString> sLabelGroupMap;
    const ATProto::UserPreferences& mUserPreferences;
    std::unordered_map<QString, ContentGroupMap> mLabelerGroupMap; // labeler DID -> group map
};

class ContentFilterShowAll : public IContentFilter
{
public:
    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::LabelList&) const override
    {
        return {QEnums::CONTENT_VISIBILITY_SHOW, ""};
    }
};

}
