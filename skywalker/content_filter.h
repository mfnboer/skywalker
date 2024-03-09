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
    Q_PROPERTY(QString subTitle MEMBER mSubTitle CONSTANT FINAL)
    Q_PROPERTY(bool isAdult MEMBER mAdultImages CONSTANT FINAL)
    QML_VALUE_TYPE(contentgroup)

public:
    QString mId;
    QString mTitle;
    QString mSubTitle;
    QString mWarning;
    std::vector<QString> mLabelValues;
    bool mAdultImages;
    QEnums::ContentVisibility mDefaultVisibility;

    bool isPostLevel() const { return mDefaultVisibility == QEnums::CONTENT_VISIBILITY_WARN_POST ||
                                      mDefaultVisibility == QEnums::CONTENT_VISIBILITY_HIDE_POST; }

    QEnums::ContentVisibility getContentVisibility(ATProto::UserPreferences::LabelVisibility visibility) const;
};

class ContentFilter : public IContentFilter
{
public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::Ptr>;
    using ContentGroupMap = std::unordered_map<QString, const ContentGroup*>;

    static const std::vector<ContentGroup> CONTENT_GROUP_LIST;
    static const ContentGroupMap& getContentGroups();

    // This function removes neg-labels, i.e. if X and not-X are labels, then X is not in the result.
    static ContentLabelList getContentLabels(const LabelList& labels);

    explicit ContentFilter(const ATProto::UserPreferences& userPreferences);

    QEnums::ContentVisibility getGroupVisibility(const QString& groupId) const;
    QEnums::ContentVisibility getVisibility(const QString& label) const;
    QString getWarning(const QString& label) const;

    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ATProto::ComATProtoLabel::LabelList& labels) const override;
    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const ContentLabelList& contentLabels) const;

private:
    static ContentGroupMap CONTENT_GROUPS;
    static void initContentGroups();
    static void initLabelGroupMap();

    static std::unordered_map<QString, QString> sLabelGroupMap;
    const ATProto::UserPreferences& mUserPreferences;
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
