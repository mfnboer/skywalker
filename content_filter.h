// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/user_preferences.h>
#include <atproto/lib/lexicon/com_atproto_label.h>
#include <QStringList>

namespace Skywalker {

struct ContentGroup
{
    QString mId;
    QString mTitle;
    QString mSubTitle;
    QString mWarning;
    std::vector<QString> mLabelValues;
    bool mAdultImages;
    ATProto::UserPreferences::LabelVisibility mDefaultVisibility;
};

class ContentFilter {
public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::Ptr>;
    using Visibility = ATProto::UserPreferences::LabelVisibility;

    static const std::unordered_map<QString, ContentGroup> CONTENT_GROUPS;

    static QStringList getLabelTexts(const LabelList& labels);

    explicit ContentFilter(ATProto::UserPreferences& userPreferences);

    Visibility getVisibility(const QString& label) const;
    QString getWarning(const QString& label) const;

private:
    static void initLabelGroupMap();

    static std::unordered_map<QString, QString> sLabelGroupMap;
    ATProto::UserPreferences& mUserPreferences;
};

}
