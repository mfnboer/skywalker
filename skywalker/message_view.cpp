// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "message_view.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

// TODO: reactions
MessageView::MessageView(const ATProto::ChatBskyConvo::MessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mText(msg.mText),
    mFormattedText(ATProto::RichTextMaster::getFormattedMessageText(msg, UserSettings::getCurrentLinkColor())),
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

// TODO: reaction
MessageView::MessageView(const ATProto::ChatBskyConvo::MessageAndReactionView& msg) :
    mId(msg.mMessageView->mId),
    mRev(msg.mMessageView->mRev),
    mText(msg.mMessageView->mText),
    mFormattedText(ATProto::RichTextMaster::getFormattedMessageText(*msg.mMessageView, UserSettings::getCurrentLinkColor())),
    mEmbed(msg.mMessageView->mEmbed),
    mSenderDid(msg.mMessageView->mSender->mDid),
    mSentAt(msg.mMessageView->mSentAt)
{
}

MessageView::MessageView(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageType& msg)
{
    if (ATProto::isNullVariant(msg))
    {
        qWarning() << "Unknown message type";
        return;
    }

    const auto* messageView = std::get_if<ATProto::ChatBskyConvo::MessageView::SharedPtr>(&msg);

    if (messageView)
    {
        init(*messageView);
        return;
    }

    const auto* deletedMessageView = std::get_if<ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr>(&msg);

    if (deletedMessageView)
    {
        init(*deletedMessageView);
        return;
    }

    Q_ASSERT(false);
    qWarning() << "Should not get here";
}

MessageView::MessageView(const ATProto::ChatBskyConvo::ConvoView::MessageType& msg)
{
    if (ATProto::isNullVariant(msg))
    {
        qWarning() << "Unknown message type";
        return;
    }

    const auto* messageView = std::get_if<ATProto::ChatBskyConvo::MessageView::SharedPtr>(&msg);

    if (messageView)
    {
        init(*messageView);
        return;
    }

    const auto* deletedMessageView = std::get_if<ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr>(&msg);

    if (deletedMessageView)
    {
        init(*deletedMessageView);
        return;
    }

    const auto* messageAndReactionView = std::get_if<ATProto::ChatBskyConvo::MessageAndReactionView::SharedPtr>(&msg);

    if (messageAndReactionView)
    {
        init(*messageAndReactionView);
        return;
    }

    Q_ASSERT(false);
    qWarning() << "Should not get here";
}

// TODO: reactions
void MessageView::init(const ATProto::ChatBskyConvo::MessageView::SharedPtr& view)
{
    mId = view->mId;
    mRev = view->mRev;
    mText = view->mText;
    mFormattedText = ATProto::RichTextMaster::getFormattedMessageText(*view, UserSettings::getCurrentLinkColor());
    mEmbed = view->mEmbed;
    mSenderDid = view->mSender->mDid;
    mSentAt = view->mSentAt;
}

void MessageView::init(const ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr& view)
{
    mId = view->mId;
    mRev = view->mRev;
    mSenderDid = view->mSender->mDid;
    mSentAt = view->mSentAt;
    mDeleted = true;
}

// TODO: reaction
void MessageView::init(const ATProto::ChatBskyConvo::MessageAndReactionView::SharedPtr& view)
{
    mId = view->mMessageView->mId;
    mRev = view->mMessageView->mRev;
    mText = view->mMessageView->mText;
    mFormattedText = ATProto::RichTextMaster::getFormattedMessageText(*view->mMessageView, UserSettings::getCurrentLinkColor());
    mEmbed = view->mMessageView->mEmbed;
    mSenderDid = view->mMessageView->mSender->mDid;
    mSentAt = view->mMessageView->mSentAt;
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
