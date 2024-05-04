// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "content_group.h"
#include "definitions.h"
#include "language_utils.h"

namespace Skywalker {

ContentGroup::ContentGroup(const QString& labelId, const QString& labelerDid) :
    mLabelId(labelId),
    mTitle(labelId),
    mLabelerDid(labelerDid)
{}

ContentGroup::ContentGroup(
    const QString& labelId, const QString& title, const QString& description,
    const std::optional<QString>& legacyLabelId, bool adult,
    QEnums::ContentVisibility defaultVisibility, QEnums::LabelTarget labelTarget,
    const QEnums::LabelSeverity& severity, const QString& labelerDid) :
    mLabelId(labelId),
    mTitle(title),
    mDescription(description),
    mLegacyLabelId(legacyLabelId),
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
}

QString ContentGroup::getTitleWithSeverity() const
{
    switch (mSeverity) {
    case QEnums::LABEL_SEVERITY_ALERT:
        return QString("⚠️ %1").arg(mTitle);
    case QEnums::LABEL_SEVERITY_INFO:
        return QString("ℹ️ %1").arg(mTitle);
    default:
        return mTitle;
    }
}

}
