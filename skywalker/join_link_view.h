// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/lexicon/chat_bsky_group.h>

namespace Skywalker {

class JoinLinkView
{
    Q_GADGET
    Q_PROPERTY(QString code READ getCode FINAL)
    Q_PROPERTY(QEnums::JoinLinkEnabledStatus enabledStatus READ getEnabledStatus FINAL)
    Q_PROPERTY(bool requireApproval READ requireApproval FINAL)
    Q_PROPERTY(QEnums::JoinRule joinRule READ getJoinRule FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    QML_VALUE_TYPE(joinlinkview)

public:
    JoinLinkView() = default;
    explicit JoinLinkView(const ATProto::ChatBskyGroup::JoinLinkView::SharedPtr& view);

    Q_INVOKABLE bool isNull() const { return mView == nullptr; }
    QString getCode() const { return mView ? mView->mCode : ""; }
    QEnums::JoinLinkEnabledStatus getEnabledStatus() const { return mView ? (QEnums::JoinLinkEnabledStatus)mView->mEnabledStatus : QEnums::JOIN_LINK_ENABLED_STATUS_DISABLED; }
    bool requireApproval() const { return mView ? mView->mRequireApproval : false; }
    QEnums::JoinRule getJoinRule() const { return mView ? (QEnums::JoinRule)mView->mJoinRule : QEnums::JOIN_RULE_ANYONE; }
    QDateTime getCreatedAt() const { return mView ? mView->mCreatedAt : QDateTime{}; }

private:
    ATProto::ChatBskyGroup::JoinLinkView::SharedPtr mView;
};

}
