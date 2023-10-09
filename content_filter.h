// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/user_preferences.h>
#include <atproto/lib/lexicon/com_atproto_label.h>
#include <QStringList>
#include <tuple>

namespace Skywalker {

struct ContentGroup
{
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

class ContentFilter {
public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::Ptr>;

    static const std::unordered_map<QString, ContentGroup> CONTENT_GROUPS;

    static QStringList getLabelTexts(const LabelList& labels);

    explicit ContentFilter(const ATProto::UserPreferences& userPreferences);

    QEnums::ContentVisibility getVisibility(const QString& label) const;
    QString getWarning(const QString& label) const;

    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const std::vector<ATProto::ComATProtoLabel::Label::Ptr>& labels) const;
    std::tuple<QEnums::ContentVisibility, QString> getVisibilityAndWarning(const QStringList& labelTexts) const;

private:
    static void initLabelGroupMap();

    static std::unordered_map<QString, QString> sLabelGroupMap;
    const ATProto::UserPreferences& mUserPreferences;
};

}
