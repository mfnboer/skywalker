// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
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
    Q_INVOKABLE bool isNull() const { return mId.isEmpty(); }

private:
    void init(const ATProto::ChatBskyConvo::MessageView::SharedPtr& view);
    void init(const ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr& view);

    QString mId;
    QString mRev;
    QString mText;
    QString mFormattedText;
    ATProto::AppBskyEmbed::RecordView::SharedPtr mEmbed;
    QString mSenderDid;
    QDateTime mSentAt;
    bool mDeleted = false;
};

using MessageViewList = QList<MessageView>;

}
