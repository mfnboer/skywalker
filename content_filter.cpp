// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_filter.h"

namespace Skywalker {

const std::unordered_map<QString, ContentGroup> ContentFilter::CONTENT_GROUPS = {
    {
        "nsfw",
        {
            "nsfw",
            "Explicit Sexual Images",
            "i.e. pornography",
            "Sexually Explicit",
            {"porn", "nsfw"},
            true,
            ATProto::UserPreferences::LabelVisibility::WARN
        }
    },
    {
        "nudity",
        {
            "nudity",
            "Other Nudity",
            "Including non-sexual and artistic",
            "Nudity",
            {"nudity"},
            true,
            ATProto::UserPreferences::LabelVisibility::WARN
        }
    },
    {
        "suggestive",
        {
            "suggestive",
            "Sexually Suggestive",
            "Does not include nudity",
            "Sexually Suggestive",
            {"sexual"},
            true,
            ATProto::UserPreferences::LabelVisibility::WARN
        }
    },
    {
        "gore",
        {
            "gore",
            "Violent / Bloody",
            "Gore, self-harm, torture",
            "Violence",
            {"gore", "self-harm", "torture", "nsfl", "corpse"},
            true,
            ATProto::UserPreferences::LabelVisibility::HIDE
        }
    },
    {
        "hate",
        {
            "hate",
            "Hate Group Iconography",
            "Images of terror groups, articles covering events, etc.",
            "Hate Groups",
            {"icon-kkk", "icon-nazi", "icon-intolerant", "behavior-intolerant"},
            false,
            ATProto::UserPreferences::LabelVisibility::HIDE
        }
    },
    {
         "spam",
        {
            "spam",
            "Spam",
            "Excessive unwanted interactions",
            "Spam",
            {"spam"},
            false,
            ATProto::UserPreferences::LabelVisibility::HIDE
        }
    },
    {
        "impersonation",
        {
            "impersonation",
            "Impersonation",
            "Accounts falsely claiming to be people or orgs",
            "Impersonation",
            {"impersonation"},
            false,
            ATProto::UserPreferences::LabelVisibility::WARN
        }
    }
};

std::unordered_map<QString, QString> ContentFilter::sLabelGroupMap;

ContentFilter::ContentFilter(ATProto::UserPreferences& userPreferences) :
    mUserPreferences(userPreferences)
{
    if (sLabelGroupMap.empty())
        initLabelGroupMap();
}

void ContentFilter::initLabelGroupMap()
{
    for (const auto& [id, group] : CONTENT_GROUPS)
    {
        for (const auto& label : group.mLabelValues)
            sLabelGroupMap[label] = id;
    }
}

QStringList ContentFilter::getLabelTexts(const LabelList& labels)
{
    QStringList labelTexts;

    for (const auto& label : labels)
    {
        if (!label->mNeg)
            labelTexts.append(label->mVal);
        else
            labelTexts.removeAll(label->mVal);
    }

    return labelTexts;
}

ContentFilter::Visibility ContentFilter::getVisibility(const QString& label) const
{
    auto it = sLabelGroupMap.find(label);

    if (it == sLabelGroupMap.end())
    {
        qDebug() << "Undefined label:" << label;
        return Visibility::SHOW;
    }

    const auto visibility = mUserPreferences.getLabelVisibility(it->second);

    if (visibility != Visibility::UNKNOWN)
        return visibility;

    const auto& group = CONTENT_GROUPS.at(it->second);
    return group.mDefaultVisibility;
}

QString ContentFilter::getWarning(const QString& label) const
{
    auto it = sLabelGroupMap.find(label);

    if (it == sLabelGroupMap.end())
    {
        qDebug() << "Undefined label:" << label;
        return QObject::tr("Unknown label") + QString(": %1").arg(label);
    }

    const auto& group = CONTENT_GROUPS.at(it->second);
    return group.mWarning;
}

}
