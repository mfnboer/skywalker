// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "reaction_view.h"
#include "record_view.h"
#include <atproto/lib/lexicon/chat_bsky_convo.h>

namespace Skywalker {

class MessageView
{
    Q_GADGET
    Q_PROPERTY(QString id READ getId FINAL)
    Q_PROPERTY(QString rev READ getRev FINAL)
    Q_PROPERTY(QString text READ getText FINAL)
    Q_PROPERTY(QString formattedText READ getFormattedText FINAL)
    Q_PROPERTY(RecordView embed READ getEmbed FINAL)
    Q_PROPERTY(QString senderDid READ getSenderDid FINAL)
    Q_PROPERTY(QDateTime sentAt READ getSentAt FINAL)
    Q_PROPERTY(bool deleted READ isDeleted FINAL)
    Q_PROPERTY(ReactionView::List reactions READ getReactions FINAL)
    QML_VALUE_TYPE(messageview)

public:
    MessageView() = default;
    explicit MessageView(const ATProto::ChatBskyConvo::MessageView& msg);
    explicit MessageView(const ATProto::ChatBskyConvo::DeletedMessageView& msg);
    explicit MessageView(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageType& msg);

    const QString& getId() const { return mId; }
    const QString& getRev() const { return mRev; }
    const QString& getText() const { return mText; }
    const QString& getFormattedText() const { return mFormattedText; }
    const RecordView getEmbed() const;
    const QString& getSenderDid() const { return mSenderDid; }
    const QDateTime getSentAt() const { return mSentAt; }
    bool isDeleted() const { return mDeleted; }
    const ReactionView::List& getReactions() const { return mReactions; }

    Q_INVOKABLE bool isNull() const { return mId.isEmpty(); }
    Q_INVOKABLE ReactionView::List getUniqueReactions(int maxReactions) const;

private:
    void init(const ATProto::ChatBskyConvo::MessageView::SharedPtr& view);
    void init(const ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr& view);
    void initReactions(const ATProto::ChatBskyConvo::ReactionView::List& reactions);

    QString mId;
    QString mRev;
    QString mText;
    QString mFormattedText;
    ATProto::AppBskyEmbed::RecordView::SharedPtr mEmbed;
    QString mSenderDid;
    QDateTime mSentAt;
    bool mDeleted = false;
    ReactionView::List mReactions;
};

using MessageViewList = QList<MessageView>;

}
