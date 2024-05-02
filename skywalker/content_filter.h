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

class ContentFilter : public IContentFilter
{
public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::Ptr>;
    using GlobalContentGroupMap = std::unordered_map<QString, const ContentGroup*>;

    static const std::vector<ContentGroup> CONTENT_GROUP_LIST;
    static const GlobalContentGroupMap& getContentGroups();
    static const ContentGroup* getGlobalContentGroup(const QString& labelId);
    static bool isGlobalLabel(const QString& labelId);

    // This function removes neg-labels, i.e. if X and not-X are labels, then X is not in the result.
    static ContentLabelList getContentLabels(const LabelList& labels);

    explicit ContentFilter(const ATProto::UserPreferences& userPreferences);

    QEnums::ContentVisibility getGroupVisibility(const ContentGroup& group) const;
    QEnums::ContentVisibility getVisibility(const ContentLabel& label) const;
    QString getGroupWarning(const ContentGroup& group) const;
    QString getWarning(const ContentLabel& label) const;

    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::LabelList& labels) const override;
    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ContentLabelList& contentLabels) const;

    bool isSubscribedToLabeler(const QString& did) const;
    static bool isFixedLabelerSubscription(const QString& did);

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
