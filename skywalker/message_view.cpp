// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "message_view.h"
#include "unicode_fonts.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

MessageView::MessageView(const ATProto::ChatBskyConvo::MessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mText(msg.mText),
    mFormattedText(ATProto::RichTextMaster::getFormattedMessageText(msg, UserSettings::getCurrentLinkColor())),
    mTextMetaInfo(UnicodeFonts::getTextMetaInfo(mText, !msg.mFacets.empty())),
    mEmbed(msg.mEmbed),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt),
    mReplyTo(msg.mReplyTo)
{
    initReactions(msg.mReactions);
}

MessageView::MessageView(const ATProto::ChatBskyConvo::DeletedMessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt),
    mDeleted(true)
{
}

MessageView::MessageView(const ATProto::ChatBskyConvo::MessageBeforeUserJoinedGroupView&) :
    mText(QObject::tr("Message from before you joined")),
    mFormattedText(ATProto::RichTextMaster::plainToHtml(mText))
{
}

MessageView::MessageView(const ATProto::UnknownVariant& msg)
{
    init(msg);
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

    const auto* systemMessageView = std::get_if<ATProto::ChatBskyConvo::SystemMessageView::SharedPtr>(&msg);

    if (systemMessageView)
    {
        init(*systemMessageView);
        return;
    }

    const auto* unknownView = std::get_if<ATProto::UnknownVariant::SharedPtr>(&msg);

    if (unknownView)
    {
        init(**unknownView);
        return;
    }

    Q_ASSERT(false);
    qWarning() << "Should not get here";
}

void MessageView::init(const ATProto::ChatBskyConvo::MessageView::SharedPtr& view)
{
    mId = view->mId;
    mRev = view->mRev;
    mText = view->mText;
    mFormattedText = ATProto::RichTextMaster::getFormattedMessageText(*view, UserSettings::getCurrentLinkColor());
    mTextMetaInfo = UnicodeFonts::getTextMetaInfo(mText, !view->mFacets.empty());
    mEmbed = view->mEmbed;
    mSenderDid = view->mSender->mDid;
    mSentAt = view->mSentAt;
    mEmbed = view->mEmbed;
    mReplyTo = view->mReplyTo;
    initReactions(view->mReactions);
}

void MessageView::init(const ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr& view)
{
    mId = view->mId;
    mRev = view->mRev;
    mSenderDid = view->mSender->mDid;
    mSentAt = view->mSentAt;
    mDeleted = true;
}

void MessageView::init(const ATProto::ChatBskyConvo::SystemMessageView::SharedPtr& view)
{
    mId = view->mId;
    mRev = view->mRev;
    mSentAt = view->mSentAt;
    mSystemMessageView = view;
}

void MessageView::init(const ATProto::UnknownVariant& view)
{
    ATProto::XJsonObject xjson(view.mJson);
    mId = xjson.getOptionalString("id", "");
    mRev = xjson.getOptionalString("rev", "");
    mSentAt = xjson.getOptionalDateTime("sentAt", QDateTime{});
    mText = QString("Unknown message: %1").arg(view.mType);
    mFormattedText = ATProto::RichTextMaster::plainToHtml(mText);
}

void MessageView::initReactions(const ATProto::ChatBskyConvo::ReactionView::List& reactions)
{
    for (const auto& reaction : reactions)
    {
        const ReactionView reactionView(reaction);
        mReactions.push_back(reactionView);
    }
}

ReactionView::List MessageView::getUniqueReactions(int maxReactions) const
{
    ReactionView::List result;
    std::unordered_set<QString> seenEmoji;

    for (const auto& reaction : mReactions)
    {
        if (seenEmoji.contains(reaction.getEmoji()))
            continue;

        result.push_back(reaction);

        if (result.size() >= maxReactions)
            break;

        seenEmoji.insert(reaction.getEmoji());
    }

    return result;
}

QEnums::MessageEmbedType MessageView::getEmbedType() const
{
    if (!mEmbed)
        return QEnums::MESSAGE_EMBED_NONE;

    if (ATProto::holdsNonNull<ATProto::AppBskyEmbed::RecordView::SharedPtr>(*mEmbed))
        return QEnums::MESSAGE_EMBED_RECORD;

    if (ATProto::holdsNonNull<ATProto::ChatBskyEmbed::JoinLinkView::SharedPtr>(*mEmbed))
        return QEnums::MESSAGE_EMBED_JOIN_LINK;

    qWarning() << "Unknown embed type";
    Q_ASSERT(false);
    return QEnums::MESSAGE_EMBED_NONE;
}

const RecordView MessageView::getEmbedRecord() const
{
    if (getEmbedType() != QEnums::MESSAGE_EMBED_RECORD)
        return {};

    const auto view = std::get<ATProto::AppBskyEmbed::RecordView::SharedPtr>(*mEmbed);

    if (!view)
    {
        qWarning() << "No record view";
        return {};
    }

    RecordView recordView{*view};
    recordView.setContentVisibility(QEnums::CONTENT_VISIBILITY_SHOW);
    recordView.setContentWarning("");
    return recordView;
}

const JoinLinkPreview MessageView::getEmbedJoinLink() const
{
    if (getEmbedType() != QEnums::MESSAGE_EMBED_JOIN_LINK)
        return {};

    const auto view = std::get<ATProto::ChatBskyEmbed::JoinLinkView::SharedPtr>(*mEmbed);

    if (!view)
    {
        qWarning() << "No join link view";
        return {};
    }

    const JoinLinkPreview joinLink =
        std::visit([](auto&& x){ return JoinLinkPreview(*x); }, view->mJoinLinkPreview);

    return joinLink;
}

MessageView MessageView::getReplyTo() const
{
    if (!mReplyTo)
        return {};

    return std::visit([](auto&& view){ return MessageView(*view); }, *mReplyTo);
}

}
