// License: GPLv3
#include "notification_content.h"
#include "post_cache.h"
#include <QObject>
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {
namespace {

template<class Entity>
QString getEntityAttachmentNotificationText(const Entity& entity)
{
    const auto images = entity.getImages();

    for (const auto& image : images)
    {
        const auto altText = image.getAlt();

        if (!altText.isEmpty())
            return ATProto::RichTextMaster::plainToHtml(altText);
    }

    const auto video = entity.getVideoView();

    if (video)
    {
        const auto altText = video->getAlt();

        if (!altText.isEmpty())
            return ATProto::RichTextMaster::plainToHtml(altText);
    }

    const auto external = entity.getExternalView();

    if (external)
    {
        const auto title = external->getTitle();

        if (!title.isEmpty())
            return ATProto::RichTextMaster::plainToHtml(title);

        const auto description = external->getDescription();

        if (!description.isEmpty())
            return ATProto::RichTextMaster::plainToHtml(description);
    }

    return {};
}

template<class Entity>
QString getEntityNotificationText(const Entity& entity)
{
    const auto text = entity.getFormattedText();

    if (!text.isEmpty())
        return text;

    return getEntityAttachmentNotificationText(entity);
}

QString getNotificationText(const Post& post)
{
    const auto postText = getEntityNotificationText(post);

    if (!postText.isEmpty())
        return postText;

    const auto record = post.getRecordView();

    if (record)
    {
        const auto recordText = getEntityNotificationText(*record);

        if (!recordText.isEmpty())
            return recordText;
    }

    const auto recordWithMedia = post.getRecordWithMediaView();

    if (recordWithMedia)
    {
        const auto recordText = getEntityAttachmentNotificationText(*recordWithMedia);

        if (!recordText.isEmpty())
            return recordText;

        const auto record = recordWithMedia->getRecord();

        if (!record.isNull())
        {
            const auto recordText = getEntityNotificationText(record);

            if (!recordText.isEmpty())
                return recordText;
        }
    }

    return {};
}

QString getNotificationText(const PostRecord& postRecord)
{
    const QString text = postRecord.getFormattedText();

    if (!text.isEmpty())
        return text;

    const auto& images = postRecord.getImages();

    if (images)
    {
        for (const auto& image : images->mImages)
        {
            if (!image->mAlt.isEmpty())
                return image->mAlt;
        }
    }

    const auto& video = postRecord.getVideo();

    if (video && !video->mAlt.value_or("").isEmpty())
        return *video->mAlt;

    const auto& external = postRecord.getExternal();

    if (external && external->mExternal)
    {
        if (!external->mExternal->mTitle.isEmpty())
            return external->mExternal->mTitle;

        if (!external->mExternal->mDescription.isEmpty())
            return external->mExternal->mDescription;
    }

    return {};
}

}

std::optional<QString> getNotificationBody(const Notification& notification, const PostCache& reasonPostCache)
{
    const PostRecord postRecord = notification.getPostRecord();
    QString body = !postRecord.isNull() ? getNotificationText(postRecord) : "";

    switch (notification.getReason())
    {
    case Notification::Reason::NOTIFICATION_REASON_LIKE:
    case Notification::Reason::NOTIFICATION_REASON_LIKE_VIA_REPOST:
    {
        body = notification.getReason() == Notification::Reason::NOTIFICATION_REASON_LIKE ?
                QObject::tr("<b>Liked your post</b>") :
                QObject::tr("<b>Liked your repost</b>");
        const auto reasonPostText = getNotificationText(notification.getReasonPost(reasonPostCache));

        if (!reasonPostText.isEmpty())
            body.append("<br>" + reasonPostText);

        break;
    }
    case Notification::Reason::NOTIFICATION_REASON_REPOST:
    case Notification::Reason::NOTIFICATION_REASON_REPOST_VIA_REPOST:
    {
        body = notification.getReason() == Notification::Reason::NOTIFICATION_REASON_REPOST ?
                QObject::tr("<b>Reposted your post</b>") :
                QObject::tr("<b>Reposted your repost</b>");
        const auto reasonPostText = getNotificationText(notification.getReasonPost(reasonPostCache));

        if (!reasonPostText.isEmpty())
            body.append("<br>" + reasonPostText);

        break;
    }
    case Notification::Reason::NOTIFICATION_REASON_FOLLOW:
        body = QObject::tr("<b>Follows you</b>");
        break;
    case Notification::Reason::NOTIFICATION_REASON_MENTION:
        body = QObject::tr("<b>Mentioned you</b><br>") + body;
        break;
    case Notification::Reason::NOTIFICATION_REASON_REPLY:
        body = QObject::tr("<b>Replied to your post</b><br>") + body;
        break;
    case Notification::Reason::NOTIFICATION_REASON_QUOTE:
        body = QObject::tr("<b>Quoted your post</b><br>") + body;
        break;
    case Notification::Reason::NOTIFICATION_REASON_SUBSCRIBED_POST:
        body = QObject::tr("<b>Subscribed post</b><br>") + body;
        break;
    case Notification::Reason::NOTIFICATION_REASON_DIRECT_MESSAGE:
    {
        const QString title = notification.getChatGroupTitle().isEmpty() ?
                QObject::tr("Direct message") : notification.getChatGroupTitle();
        body = QObject::tr("<b>%1</b><br>").arg(title) + notification.getDirectMessage().getFormattedText();
        break;
    }
    case Notification::Reason::NOTIFICATION_REASON_DIRECT_MESSAGE_REACTION:
    {
        const QString title = notification.getChatGroupTitle().isEmpty() ?
                QObject::tr("Direct message") : notification.getChatGroupTitle();
        const auto& msgAndReaction = notification.getDirectMessageAndReaction();
        body = QObject::tr("<b>%1</b><br>Reaction %2 to: %3").arg(
                title,
                msgAndReaction.getReactionView().getEmoji(),
                msgAndReaction.getMessageView().getFormattedText());
        break;
    }
    case Notification::Reason::NOTIFICATION_REASON_CHAT_JOIN_REQUEST:
        body = QObject::tr("<b>Join request</b><br>") + notification.getJoinRequest().getTitle();
        break;
    case Notification::Reason::NOTIFICATION_REASON_VERIFIED:
        body = QObject::tr("<b>Verified you</b>");
        break;
    case Notification::Reason::NOTIFICATION_REASON_UNVERIFIED:
        body = QObject::tr("<b>Deleted your verification</b>");
        break;
    case Notification::Reason::NOTIFICATION_REASON_STARTERPACK_JOINED:
    case Notification::Reason::NOTIFICATION_REASON_INVITE_CODE_USED:
    case Notification::Reason::NOTIFICATION_REASON_NEW_LABELS:
    case Notification::Reason::NOTIFICATION_REASON_UNKNOWN:
        return {};
    }

    return body;
}

}
