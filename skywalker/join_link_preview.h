// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "chat_profile.h"
#include "enums.h"
#include <atproto/lib/lexicon/chat_bsky_group.h>

namespace Skywalker {

class JoinLinkPreview
{
    Q_GADGET
    Q_PROPERTY(QString convoId READ getConvoId FINAL)
    Q_PROPERTY(QString code READ getCode FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(ChatBasicProfile owner READ getOwner FINAL)
    Q_PROPERTY(int memberCount READ getMemberCount FINAL)
    Q_PROPERTY(int memberLimit READ getMemberLimit FINAL)
    Q_PROPERTY(bool requireApproval READ requireApproval FINAL)
    Q_PROPERTY(QEnums::JoinRule joinRule READ getJoinRule FINAL)
    Q_PROPERTY(bool userIsMember READ userIsMember FINAL)
    Q_PROPERTY(bool requestPending READ isRequestPending FINAL)
    Q_PROPERTY(QDateTime requestedAt READ getRequestedAt FINAL)
    Q_PROPERTY(bool disabled READ isDisabled FINAL)
    Q_PROPERTY(bool invalid READ isInvalid FINAL)
    QML_VALUE_TYPE(joinlinkpreview)

public:
    JoinLinkPreview() = default;
    JoinLinkPreview(const ATProto::ChatBskyGroup::JoinLinkPreviewView& view);
    JoinLinkPreview(const ATProto::ChatBskyGroup::DisabledJoinLinkPreviewView& view);
    JoinLinkPreview(const ATProto::ChatBskyGroup::InvalidJoinLinkPreviewView& view);
    JoinLinkPreview(const ATProto::UnknownVariant& view);

    Q_INVOKABLE bool isNull() const { return mCode.isEmpty(); }
    const QString& getConvoId() const { return mConvoId; }
    const QString& getCode() const { return mCode; }
    const QString& getName() const { return mName; }
    const ChatBasicProfile& getOwner() const { return mOwner; }
    int getMemberCount() const { return mMemberCount; }
    int getMemberLimit() const { return mMemberLimit; }
    bool requireApproval() const { return mRequireApproval; }
    QEnums::JoinRule getJoinRule() const { return mJoinRule; }
    bool userIsMember() const { return mConvoView != nullptr; }
    bool isRequestPending() const { return mRequestedAt.has_value(); }
    QDateTime getRequestedAt() const { return mRequestedAt.value_or(QDateTime{}); }

    bool isDisabled() const { return mDisabled; }
    bool isInvalid() const { return mInvalid; }

    // Returns a ConvoView when present, i.e. the active user is a member of the group
    Q_INVOKABLE QVariant getConvo() const;
    void setRequestedAtToNow();
    void clearRequestedAt() { mRequestedAt = {}; }
    void setConvoView(const ATProto::ChatBskyConvo::ConvoView::SharedPtr& convo);

private:
    QString mConvoId;
    QString mCode;
    QString mName;
    ChatBasicProfile mOwner;
    int mMemberCount = 0;
    int mMemberLimit = 0;
    bool mRequireApproval = false;
    QEnums::JoinRule mJoinRule;
    ATProto::ChatBskyConvo::ConvoView::SharedPtr mConvoView;
    std::optional<QDateTime> mRequestedAt;
    bool mDisabled = false;
    bool mInvalid = false;
};

}
