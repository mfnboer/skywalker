// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_filter.h"
#include "definitions.h"

namespace Skywalker {

const std::vector<ContentGroup> ContentFilter::CONTENT_GROUP_LIST = {
    {
        "porn",
        QObject::tr("Adult Content"),
        QObject::tr("Explicit sexual images"),
        "nsfw",
        true,
        QEnums::CONTENT_VISIBILITY_WARN_MEDIA,
        QEnums::LABEL_TARGET_MEDIA
    },
    {
        "nudity",
        QObject::tr("Non-sexual Nudity"),
        QObject::tr("E.g. artistic nudes"),
        {},
        true,
        QEnums::CONTENT_VISIBILITY_WARN_MEDIA,
        QEnums::LABEL_TARGET_MEDIA
    },
    {
        "sexual",
        QObject::tr("Sexually Suggestive"),
        QObject::tr("Does not include nudity"),
        "suggestive",
        true,
        QEnums::CONTENT_VISIBILITY_WARN_MEDIA,
        QEnums::LABEL_TARGET_MEDIA
    },
    {
        "graphic-media",
        QObject::tr("Graphic Media"),
        QObject::tr("Explicit or potentially disturbing media"),
        "gore",
        true,
        QEnums::CONTENT_VISIBILITY_HIDE_MEDIA,
        QEnums::LABEL_TARGET_MEDIA
    }
};

ContentFilter::GlobalContentGroupMap ContentFilter::CONTENT_GROUPS;

ContentGroup::ContentGroup(
        const QString& labelId, const QString& title, const QString& description,
        const std::optional<QString>& legacyLabelId, bool adult,
        QEnums::ContentVisibility defaultVisibility, QEnums::LabelTarget labelTarget) :
    mLabelId(labelId),
    mTitle(title),
    mDescription(description),
    mLegacyLabelId(legacyLabelId),
    mAdult(adult),
    mDefaultVisibility(defaultVisibility),
    mLabelTarget(labelTarget)
{
}

ContentGroup::ContentGroup(const ATProto::ComATProtoLabel::LabelValueDefinition& labelDef) :
    mLabelId(labelDef.mIdentifier),
    mAdult(labelDef.mAdultOnly)
{
    switch (labelDef.mBlurs)
    {
    case ATProto::ComATProtoLabel::LabelValueDefinition::Blurs::CONTENT:
    case ATProto::ComATProtoLabel::LabelValueDefinition::Blurs::NONE:
    case ATProto::ComATProtoLabel::LabelValueDefinition::Blurs::UNKNOWN:
        mLabelTarget = QEnums::LABEL_TARGET_CONTENT;
        break;
    case ATProto::ComATProtoLabel::LabelValueDefinition::Blurs::MEDIA:
        mLabelTarget = QEnums::LABEL_TARGET_MEDIA;
        break;
    }

    switch (labelDef.mDefaultSetting)
    {
    case ATProto::ComATProtoLabel::LabelValueDefinition::Setting::IGNORE:
    case ATProto::ComATProtoLabel::LabelValueDefinition::Setting::UNKNOWN:
        mDefaultVisibility = QEnums::CONTENT_VISIBILITY_SHOW;
        break;
    case ATProto::ComATProtoLabel::LabelValueDefinition::Setting::WARN:
        mDefaultVisibility = isPostLevel() ? QEnums::CONTENT_VISIBILITY_WARN_POST : QEnums::CONTENT_VISIBILITY_WARN_MEDIA;
        break;
    case ATProto::ComATProtoLabel::LabelValueDefinition::Setting::HIDE:
        mDefaultVisibility = isPostLevel() ? QEnums::CONTENT_VISIBILITY_HIDE_POST : QEnums::CONTENT_VISIBILITY_HIDE_MEDIA;
        break;
    }

    for (const auto& locale : labelDef.mLocales)
    {
        // TODO: improve language matching, e.g. en_US, en_UK, en
        if (locale->mLang == UI_LANGUAGE)
        {
            mTitle = locale->mName;
            mDescription = locale->mDescription;
            break;
        }
    }

    if (mTitle.isEmpty() && !labelDef.mLocales.empty())
    {
        mTitle = labelDef.mLocales.front()->mLang;
        mDescription = labelDef.mLocales.front()->mDescription;
    }
}

void ContentFilter::initContentGroups()
{
    for (const auto& group : CONTENT_GROUP_LIST)
        CONTENT_GROUPS[group.mLabelId] = &group;
}

const ContentFilter::GlobalContentGroupMap& ContentFilter::getContentGroups()
{
    if (CONTENT_GROUPS.empty())
        initContentGroups();

    return CONTENT_GROUPS;
}

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

ContentFilter::ContentFilter(const ATProto::UserPreferences& userPreferences) :
    mUserPreferences(userPreferences)
{
    if (sLabelGroupMap.empty())
        initLabelGroupMap();
}

void ContentFilter::initLabelGroupMap()
{
    for (const auto& [id, group] : getContentGroups())
    {
        sLabelGroupMap[id] = id;

        if (group->mLegacyLabelId)
            sLabelGroupMap[*group->mLegacyLabelId] = id;
    }
}

ContentLabelList ContentFilter::getContentLabels(const LabelList& labels)
{
    ContentLabelList contentLabels;

    for (const auto& label : labels)
    {
        const ContentLabel contentLabel(label->mSrc, label->mVal, label->mCreatedAt);

        if (!label->mNeg)
        {
            contentLabels.append(contentLabel);
        }
        else
        {
            contentLabels.removeIf([&contentLabel](const ContentLabel& l){
                return l.getText() == contentLabel.getText() && l.getDid() == contentLabel.getDid();
            });
        }
    }

    return contentLabels;
}

QEnums::ContentVisibility ContentFilter::getGroupVisibility(const ContentGroup& group) const
{


    if (group.mAdult && !mUserPreferences.getAdultContent())
        return QEnums::CONTENT_VISIBILITY_HIDE_MEDIA;

    auto visibility = mUserPreferences.getLabelVisibility(group.mLabelId);

    if (visibility == ATProto::UserPreferences::LabelVisibility::UNKNOWN && group.mLegacyLabelId)
        visibility = mUserPreferences.getLabelVisibility(*group.mLegacyLabelId);

    if (visibility != ATProto::UserPreferences::LabelVisibility::UNKNOWN)
        return group.getContentVisibility(visibility);

    return group.mDefaultVisibility;
}

QEnums::ContentVisibility ContentFilter::getVisibility(const ContentLabel& label) const
{
    auto it = sLabelGroupMap.find(label.getText());

    if (it != sLabelGroupMap.end())
    {
        const auto& groupId = it->second;
        const auto& group = getContentGroups().at(groupId);
        return getGroupVisibility(*group);
    }

    auto labelerIt = mLabelerGroupMap.find(label.getDid());

    if (labelerIt != mLabelerGroupMap.end())
    {
        const auto& groupMap = labelerIt->second;
        auto groupIt = groupMap.find(label.getText());

        if (groupIt != groupMap.end())
            return getGroupVisibility(groupIt->second);
    }


    qDebug() << "Undefined label:" << label.getText() << "labeler:" << label.getDid();
    return QEnums::CONTENT_VISIBILITY_SHOW;
}

QString ContentFilter::getGroupWarning(const ContentGroup& group) const
{
    if (group.mAdult && !mUserPreferences.getAdultContent())
        return QObject::tr("Adult content");

    return group.mTitle;
}

QString ContentFilter::getWarning(const ContentLabel& label) const
{
    auto it = sLabelGroupMap.find(label.getText());

    if (it != sLabelGroupMap.end())
    {
        const auto& groupId = it->second;
        const auto& group = getContentGroups().at(groupId);
        return getGroupWarning(*group);
    }

    auto labelerIt = mLabelerGroupMap.find(label.getDid());

    if (labelerIt != mLabelerGroupMap.end())
    {
        const auto& groupMap = labelerIt->second;
        auto groupIt = groupMap.find(label.getText());

        if (groupIt != groupMap.end())
            return getGroupWarning(groupIt->second);
    }

    qDebug() << "Undefined label:" << label.getText() << "labeler:" << label.getDid();
    return QObject::tr("Unknown label") + QString(": %1").arg(label.getText());
}

std::tuple<QEnums::ContentVisibility, QString> ContentFilter::getVisibilityAndWarning(const ATProto::ComATProtoLabel::LabelList& labels) const
{
    const auto contentLabels = getContentLabels(labels);
    return getVisibilityAndWarning(contentLabels);
}

std::tuple<QEnums::ContentVisibility, QString> ContentFilter::getVisibilityAndWarning(const ContentLabelList& contentLabels) const
{
    QEnums::ContentVisibility visibility = QEnums::CONTENT_VISIBILITY_SHOW;
    QString warning;

    for (const auto& label : contentLabels)
    {
        const auto v = getVisibility(label);

        if (v <= visibility)
            continue;

        visibility = v;
        warning = getWarning(label);
    }

    return {visibility, warning};
}

}
