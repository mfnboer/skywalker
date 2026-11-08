// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "ozone_policy_view.h"
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneActionView
{
    Q_GADGET
    Q_PROPERTY(int id READ getId FINAL)
    Q_PROPERTY(QString type READ getType FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(QDateTime reversedAt READ getReversedAt FINAL)
    Q_PROPERTY(QDateTime expiresAt READ getExpiresAt FINAL)
    Q_PROPERTY(QStringList labels READ getLabels FINAL)
    Q_PROPERTY(OzonePolicyView::List policies READ getPolicies FINAL)
    QML_VALUE_TYPE(ozoneactionview)

public:
    using List = QList<OzoneActionView>;

    static List makeList(const ATProto::ToolsOzoneInbox::ActionView::List& actions);

    OzoneActionView() = default;
    explicit OzoneActionView(const ATProto::ToolsOzoneInbox::ActionView::SharedPtr& action);

    Q_INVOKABLE bool isNull() const { return mAction == nullptr; }
    int getId() const { return mAction ? mAction->mId : -1; }
    QString getType() const { return mAction ? mAction->mType : ""; }
    QDateTime getCreatedAt() const { return mAction ? mAction->mCreatedAt : QDateTime{}; }
    QDateTime getReversedAt() const { return (mAction && mAction->mReversedAt) ? *mAction->mReversedAt : QDateTime{}; }
    QDateTime getExpiresAt() const { return (mAction && mAction->mExpiresAt) ? *mAction->mExpiresAt : QDateTime{}; }
    QStringList getLabels() const { return mAction ? mAction->mLabels : QStringList{}; }
    OzonePolicyView::List getPolicies() const { return mAction ? OzonePolicyView::makeList(mAction->mPolicies) : OzonePolicyView::List{}; }

private:
    ATProto::ToolsOzoneInbox::ActionView::SharedPtr mAction;
};

}