// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_filter.h"

namespace Skywalker {

const std::unordered_map<QString, ContentGroup> ContentFilter::CONTENT_GROUPS = {
    {
        "nsfw",
        {
            "nsfw",
            QObject::tr("Explicit Sexual Images"),
            QObject::tr("i.e. pornography"),
            QObject::tr("Sexually Explicit"),
            {"porn", "nsfw"},
            true,
      QEnums::CONTENT_VISIBILITY_WARN_MEDIA
        }
    },
    {
        "nudity",
        {
            "nudity",
            QObject::tr("Other Nudity"),
            QObject::tr("Including non-sexual and artistic"),
            QObject::tr("Nudity"),
            {"nudity"},
            true,
      QEnums::CONTENT_VISIBILITY_WARN_MEDIA
        }
    },
    {
        "suggestive",
        {
            "suggestive",
            QObject::tr("Sexually Suggestive"),
            QObject::tr("Does not include nudity"),
            QObject::tr("Sexually Suggestive"),
            {"sexual"},
            true,
      QEnums::CONTENT_VISIBILITY_WARN_MEDIA
        }
    },
    {
        "gore",
        {
            "gore",
            QObject::tr("Violent / Bloody"),
            QObject::tr("Gore, self-harm, torture"),
            QObject::tr("Violence"),
            {"gore", "self-harm", "torture", "nsfl", "corpse"},
            true,
      QEnums::CONTENT_VISIBILITY_HIDE_MEDIA
        }
    },
    {
        "hate",
        {
            "hate",
            QObject::tr("Hate Group Iconography"),
            QObject::tr("Images of terror groups, articles covering events, etc."),
            QObject::tr("Hate Groups"),
            {"icon-kkk", "icon-nazi", "icon-intolerant", "behavior-intolerant"},
            false,
      QEnums::CONTENT_VISIBILITY_HIDE_POST
        }
    },
    {
         "spam",
        {
            "spam",
            QObject::tr("Spam"),
            QObject::tr("Excessive unwanted interactions"),
            QObject::tr("Spam"),
            {"spam"},
            false,
      QEnums::CONTENT_VISIBILITY_HIDE_POST
        }
    },
    {
        "impersonation",
        {
            "impersonation",
            QObject::tr("Impersonation"),
            QObject::tr("Accounts falsely claiming to be people or orgs"),
            QObject::tr("Impersonation"),
            {"impersonation"},
            false,
      QEnums::CONTENT_VISIBILITY_WARN_POST
        }
    }
};

QEnums::ContentVisibility ContentGroup::getContentVisibility(ATProto::UserPreferences::LabelVisibility visibility) const
{
    switch (visibility)
    {
    case ATProto::UserPreferences::LabelVisibility::SHOW:
        return QEnums::CONTENT_VISIBILITY_SHOW;
    case ATProto::UserPreferences::LabelVisibility::WARN:
        return isPostLevel() ? QEnums::CONTENT_VISIBILITY_WARN_POST : QEnums::CONTENT_VISIBILITY_WARN_MEDIA;
    case ATProto::UserPreferences::LabelVisibility::HIDE:
        return isPostLevel() ? QEnums::CONTENT_VISIBILITY_HIDE_POST : QEnums::CONTENT_VISIBILITY_HIDE_MEDIA;
    case ATProto::UserPreferences::LabelVisibility::UNKNOWN:
        Q_ASSERT(false);
        return QEnums::CONTENT_VISIBILITY_SHOW;
    }

    Q_ASSERT(false);
    return QEnums::CONTENT_VISIBILITY_SHOW;
}

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

QEnums::ContentVisibility ContentFilter::getVisibility(const QString& label) const
{
    auto it = sLabelGroupMap.find(label);

    if (it == sLabelGroupMap.end())
    {
        qDebug() << "Undefined label:" << label;
        return QEnums::CONTENT_VISIBILITY_SHOW;
    }

    const auto& group = CONTENT_GROUPS.at(it->second);

    if (group.mAdultImages && !mUserPreferences.getAdultContent())
        return QEnums::CONTENT_VISIBILITY_HIDE_MEDIA;

    const auto visibility = mUserPreferences.getLabelVisibility(it->second);

    if (visibility != ATProto::UserPreferences::LabelVisibility::UNKNOWN)
        return group.getContentVisibility(visibility);

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

    if (group.mAdultImages && !mUserPreferences.getAdultContent())
        return QObject::tr("Adult content");

    return group.mWarning;
}

std::tuple<QEnums::ContentVisibility, QString> ContentFilter::getVisibilityAndWarning(const std::vector<ATProto::ComATProtoLabel::Label::Ptr>& labels) const
{
    QEnums::ContentVisibility visibility = QEnums::CONTENT_VISIBILITY_SHOW;
    QString warning;
    const auto labelTexts = getLabelTexts(labels);

    for (const auto& labelText : labelTexts)
    {
        const auto v = getVisibility(labelText);

        if (v <= visibility)
            continue;

        visibility = v;
        warning = getWarning(labelText);
    }

    return {visibility, warning};
}

}
