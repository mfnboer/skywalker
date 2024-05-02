// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "content_group.h"
#include "definitions.h"

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
    const QString& labelerDid) :
    mLabelId(labelId),
    mTitle(title),
    mDescription(description),
    mLegacyLabelId(legacyLabelId),
    mAdult(adult),
    mDefaultVisibility(defaultVisibility),
    mLabelTarget(labelTarget),
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

}
