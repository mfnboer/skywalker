// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/chat_bsky_convo.h>

namespace Skywalker {

class MessageView
{
    Q_GADGET
    Q_PROPERTY(QString id READ getId FINAL)
    Q_PROPERTY(QString rev READ getRev FINAL)
    Q_PROPERTY(QString text READ getText FINAL)
    Q_PROPERTY(QString formattedText READ getFormattedText FINAL)
    Q_PROPERTY(QString senderDid READ getSenderDid FINAL)
    Q_PROPERTY(QDateTime sentAt READ getSentAt FINAL)
    QML_VALUE_TYPE(messageview)

public:
    MessageView() = default;
    explicit MessageView(const ATProto::ChatBskyConvo::MessageView& msg);

    const QString& getId() const { return mId; }
    const QString& getRev() const { return mRev; }
    const QString& getText() const { return mText; }
    const QString& getFormattedText() const { return mFormattedText; }
    const QString& getSenderDid() const { return mSenderDid; }
    const QDateTime getSentAt() const { return mSentAt; }

private:
    QString mId;
    QString mRev;
    QString mText;
    QString mFormattedText;
    // TODO embed
    QString mSenderDid;
    QDateTime mSentAt;
};

}
