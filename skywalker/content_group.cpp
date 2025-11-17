// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "content_group.h"
#include "content_filter.h"
#include "definitions.h"
#include "language_utils.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

ContentGroup::ContentGroup(const QString& labelId, const QString& labelerDid) :
    mLabelId(labelId),
    mTitle(labelId),
    mLabelerDid(labelerDid)
{}

ContentGroup::ContentGroup(
    const QString& labelId, const QString& title, const QString& description,
    const QStringList& legacyLabelIds, bool adult,
    QEnums::ContentVisibility defaultVisibility, QEnums::LabelTarget labelTarget,
    const QEnums::LabelSeverity& severity, const QString& labelerDid) :
    mLabelId(labelId),
    mTitle(title),
    mDescription(description),
    mLegacyLabelIds(legacyLabelIds),
    mAdult(adult),
    mDefaultVisibility(defaultVisibility),
    mLabelTarget(labelTarget),
    mSeverity(severity),
    mLabelerDid(labelerDid)
{
}

ContentGroup::ContentGroup(const ATProto::ComATProtoLabel::LabelValueDefinition& labelDef,
                           const QString& labelerDid) :
    mLabelId(labelDef.mIdentifier),
    mAdult(labelDef.mAdultOnly),
    mIsBadge(labelDef.mBlurs == ATProto::ComATProtoLabel::LabelValueDefinition::Blurs::NONE &&
               labelDef.mSeverity !=  ATProto::ComATProtoLabel::LabelValueDefinition::Severity::ALERT),
    mLabelerDid(labelerDid)
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

    switch (labelDef.mSeverity)
    {
    case ATProto::ComATProtoLabel::LabelValueDefinition::Severity::NONE:
    case ATProto::ComATProtoLabel::LabelValueDefinition::Severity::UNKNOWN:
        mSeverity = QEnums::LABEL_SEVERITY_NONE;
        break;
    case ATProto::ComATProtoLabel::LabelValueDefinition::Severity::INFORM:
        mSeverity = QEnums::LABEL_SEVERITY_INFO;
        break;
    case ATProto::ComATProtoLabel::LabelValueDefinition::Severity::ALERT:
        mSeverity = QEnums::LABEL_SEVERITY_ALERT;
        break;
    }

    const Language uiLang(UI_LANGUAGE, {});

    for (const auto& locale : labelDef.mLocales)
    {
        const auto match = uiLang.compare(Language(locale->mLang, {}));

        if (match != Language::Match::NONE && mTitle.isEmpty())
        {
            mTitle = locale->mName;
            mDescription = locale->mDescription;
        }

        // Full match on UI-language
        if (match == Language::Match::CODE)
            break;
    }

    if (mTitle.isEmpty() && !labelDef.mLocales.empty())
    {
        mTitle = labelDef.mLocales.front()->mLang;
        mDescription = labelDef.mLocales.front()->mDescription;
    }

    qDebug() << "Label:" << mLabelId << "target:" << mLabelTarget << "severity:" << mSeverity << "default:" << mDefaultVisibility << "def:" << labelDef.mRawBlurs << labelDef.mRawSeverity << labelDef.mRawDefaultSetting;
}

QString ContentGroup::getTitleWithSeverity() const
{
    switch (mSeverity) {
    case QEnums::LABEL_SEVERITY_ALERT:
        return QString("⚠️ %1").arg(mTitle);
    case QEnums::LABEL_SEVERITY_INFO:
    case QEnums::LABEL_SEVERITY_NONE:
        return mTitle;
    }

    return mTitle;
}

QString ContentGroup::getFormattedDescription() const
{
    return ATProto::RichTextMaster::plainToHtml(mDescription);
}

QEnums::ContentVisibility ContentGroup::getContentVisibility(ATProto::UserPreferences::LabelVisibility visibility) const
{
    // For a badge label, warn means show labels and content
    switch (visibility)
    {
    case ATProto::UserPreferences::LabelVisibility::SHOW:
        return QEnums::CONTENT_VISIBILITY_SHOW;
    case ATProto::UserPreferences::LabelVisibility::WARN:
        if (mIsBadge)
            return QEnums::CONTENT_VISIBILITY_SHOW;
        else
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

QEnums::ContentVisibility ContentGroup::getDefaultVisibility() const
{
    if (!mIsBadge)
        return mDefaultVisibility;

    // For a badge label, warn means show labels and content
    switch (mDefaultVisibility)
    {
    case QEnums::CONTENT_VISIBILITY_SHOW:
    case QEnums::CONTENT_VISIBILITY_WARN_MEDIA:
    case QEnums::CONTENT_VISIBILITY_WARN_POST:
        return QEnums::CONTENT_VISIBILITY_SHOW;
    case QEnums::CONTENT_VISIBILITY_HIDE_MEDIA:
        return QEnums::CONTENT_VISIBILITY_HIDE_MEDIA;
    case QEnums::CONTENT_VISIBILITY_HIDE_POST:
        return QEnums::CONTENT_VISIBILITY_HIDE_POST;
    }

    Q_ASSERT(false);
    return mDefaultVisibility;
}

bool ContentGroup::mustShowBadge(ATProto::UserPreferences::LabelVisibility visibility) const
{
    if (!mIsBadge)
        return true;

    // If visibility is "show" for a badge label, then only show content, no badge
    if (visibility == ATProto::UserPreferences::LabelVisibility::SHOW)
        return false;

    if (visibility == ATProto::UserPreferences::LabelVisibility::UNKNOWN && mDefaultVisibility == QEnums::CONTENT_VISIBILITY_SHOW)
        return false;

    return true;
}

bool ContentGroup::isSystem() const
{
    return ContentFilter::isSystemLabelId(mLabelId);
}

}
