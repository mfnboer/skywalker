// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/lexicon/com_atproto_label.h>
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ContentGroup
{
    Q_GADGET
    Q_PROPERTY(QString title MEMBER mTitle CONSTANT FINAL)
    Q_PROPERTY(QString titleWithSeverity READ getTitleWithSeverity CONSTANT FINAL)
    Q_PROPERTY(QString description MEMBER mDescription CONSTANT FINAL)
    Q_PROPERTY(QString formattedDescription READ getFormattedDescription CONSTANT FINAL)
    Q_PROPERTY(bool isAdult MEMBER mAdult CONSTANT FINAL)
    Q_PROPERTY(QEnums::LabelTarget target MEMBER mLabelTarget CONSTANT FINAL)
    Q_PROPERTY(QEnums::LabelSeverity severity MEMBER mSeverity CONSTANT FINAL)
    QML_VALUE_TYPE(contentgroup)

public:
    ContentGroup() = default;
    ContentGroup(const QString& labelId, const QString& labelerDid);
    ContentGroup(const QString& labelId, const QString& title, const QString& description,
                 const QStringList& legacyLabelIds, bool adult,
                 QEnums::ContentVisibility defaultVisibility, QEnums::LabelTarget labelTarget,
                 const QEnums::LabelSeverity& severity, const QString& labelerDid);
    explicit ContentGroup(const ATProto::ComATProtoLabel::LabelValueDefinition& labelDef,
                          const QString& labelerDid);

    const QString& getLabelId() const { return mLabelId; }
    const QString& getTitle() const { return mTitle; }
    QString getTitleWithSeverity() const;
    const QStringList& getLegacyLabelIds() const { return mLegacyLabelIds; }
    bool isAdult() const { return mAdult; }
    QEnums::ContentVisibility getDefaultVisibility() const { return mDefaultVisibility; }

    bool isPostLevel() const { return mLabelTarget == QEnums::LABEL_TARGET_CONTENT; }
    QString getFormattedDescription() const;
    QEnums::ContentVisibility getContentVisibility(ATProto::UserPreferences::LabelVisibility visibility) const;
    const QString& getLabelerDid() const { return mLabelerDid; }
    bool isGlobal() const { return mLabelerDid.isEmpty(); }

private:
    QString mLabelId;
    QString mTitle;
    QString mDescription;
    QStringList mLegacyLabelIds;
    bool mAdult = false;
    QEnums::ContentVisibility mDefaultVisibility = QEnums::ContentVisibility::CONTENT_VISIBILITY_SHOW;
    QEnums::LabelTarget mLabelTarget = QEnums::LabelTarget::LABEL_TARGET_CONTENT;
    QEnums::LabelSeverity mSeverity = QEnums::LabelSeverity::LABEL_SEVERITY_NONE;
    QString mLabelerDid; // empty means global
};

using ContentGroupList = QList<ContentGroup>;
using ContentGroupMap = std::unordered_map<QString, ContentGroup>; // label ID -> group

}

Q_DECLARE_METATYPE(::Skywalker::ContentGroup)
