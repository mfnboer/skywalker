// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "message_view.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

MessageView::MessageView(const ATProto::ChatBskyConvo::MessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mText(msg.mText),
    mFormattedText(ATProto::RichTextMaster::getFormattedMessageText(msg, UserSettings::getLinkColor())),
    mEmbed(msg.mEmbed),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt)
{
}

MessageView::MessageView(const ATProto::ChatBskyConvo::DeletedMessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt),
    mDeleted(true)
{
}

MessageView::MessageView(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageType& msg)
{
    if (ATProto::isNullVariant(msg))
    {
        qWarning() << "Unknown message type";
        return;
    }

    const auto* messageView = std::get_if<ATProto::ChatBskyConvo::MessageView::Ptr>(&msg);

    if (messageView)
    {
        const auto& view = *messageView;
        mId = view->mId;
        mRev = view->mRev;
        mText = view->mText;
        mFormattedText = ATProto::RichTextMaster::getFormattedMessageText(*view, UserSettings::getLinkColor());
        mEmbed = view->mEmbed;
        mSenderDid = view->mSender->mDid;
        mSentAt = view->mSentAt;
        return;
    }

    const auto* deletedMessageView = std::get_if<ATProto::ChatBskyConvo::DeletedMessageView::Ptr>(&msg);

    if (deletedMessageView)
    {
        const auto& view = *deletedMessageView;
        mId = view->mId;
        mRev = view->mRev;
        mSenderDid = view->mSender->mDid;
        mSentAt = view->mSentAt;
        mDeleted = true;
        return;
    }

    Q_ASSERT(false);
    qWarning() << "Should not get here";
}

const RecordView MessageView::getEmbed() const
{
    if (!mEmbed)
        return {};

    RecordView recordView{*mEmbed};
    recordView.setContentVisibility(QEnums::CONTENT_VISIBILITY_SHOW);
    recordView.setContentWarning("");
    return recordView;
}

}
