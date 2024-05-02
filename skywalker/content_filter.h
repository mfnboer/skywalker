// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "enums.h"
#include <atproto/lib/user_preferences.h>
#include <atproto/lib/lexicon/com_atproto_label.h>
#include <QStringList>
#include <QObject>
#include <QtQmlIntegration>
#include <tuple>

namespace Skywalker {

class IContentFilter
{
public:
    virtual ~IContentFilter() = default;
    virtual std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::LabelList& labels) const = 0;
};

class ContentGroup
{
    Q_GADGET
    Q_PROPERTY(QString title MEMBER mTitle CONSTANT FINAL)
    Q_PROPERTY(QString description MEMBER mDescription CONSTANT FINAL)
    Q_PROPERTY(QString formattedDescription READ getFormattedDescription CONSTANT FINAL)
    Q_PROPERTY(bool isAdult MEMBER mAdult CONSTANT FINAL)
    QML_VALUE_TYPE(contentgroup)

public:
    ContentGroup() = default;
    explicit ContentGroup(const QString& labelId);
    ContentGroup(const QString& labelId, const QString& title, const QString& description,
                 const std::optional<QString>& legacyLabelId, bool adult,
                 QEnums::ContentVisibility defaultVisibility, QEnums::LabelTarget labelTarget);
    explicit ContentGroup(const ATProto::ComATProtoLabel::LabelValueDefinition& labelDef);

    const QString& getLabelId() const { return mLabelId; }
    const QString& getTitle() const { return mTitle; }
    const std::optional<QString>& getLegacyLabelId() const { return mLegacyLabelId; }
    bool isAdult() const { return mAdult; }
    QEnums::ContentVisibility getDefaultVisibility() const { return mDefaultVisibility; }

    bool isPostLevel() const { return mLabelTarget == QEnums::LABEL_TARGET_CONTENT; }
    QString getFormattedDescription() const;
    QEnums::ContentVisibility getContentVisibility(ATProto::UserPreferences::LabelVisibility visibility) const;

private:
    QString mLabelId;
    QString mTitle;
    QString mDescription;
    std::optional<QString> mLegacyLabelId;
    bool mAdult = false;
    QEnums::ContentVisibility mDefaultVisibility = QEnums::ContentVisibility::CONTENT_VISIBILITY_SHOW;
    QEnums::LabelTarget mLabelTarget = QEnums::LabelTarget::LABEL_TARGET_CONTENT;
};

using ContentGroupList = QList<ContentGroup>;
using ContentGroupMap = std::unordered_map<QString, ContentGroup>; // label ID -> group

class ContentFilter : public IContentFilter
{
public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::Ptr>;
    using GlobalContentGroupMap = std::unordered_map<QString, const ContentGroup*>;

    static const std::vector<ContentGroup> CONTENT_GROUP_LIST;
    static const GlobalContentGroupMap& getContentGroups();
    static const ContentGroup* getGlobalContentGroup(const QString& labelId);

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

Q_DECLARE_METATYPE(::Skywalker::ContentGroup)
