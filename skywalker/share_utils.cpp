// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "share_utils.h"
#include "link_utils.h"
#include "skywalker.h"
#include <QClipboard>

namespace Skywalker {

ShareUtils::ShareUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void ShareUtils::copyUriToClipboard(const QString& uri)
{
    ATProto::ATUri atUri(uri);

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());
    copyToClipboard(shareUri);
}

void ShareUtils::sharePost(const QString& postUri, const QString& feedDid, const QString& feedContext)
{
    qDebug() << "Share post:" << postUri;
    ATProto::ATUri atUri(postUri);

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());
    share(shareUri, "Post link");

    mSkywalker->addFeedInteraction(
        feedDid,
        ATProto::AppBskyFeed::Interaction::EventType::InteractionShare,
        postUri, feedContext);
}

void ShareUtils::shareFeed(const GeneratorView& feed)
{
    qDebug() << "Share feed:" << feed.getDisplayName();
    ATProto::ATUri atUri(feed.getUri());

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());
    share(shareUri, "Feed link");
}

void ShareUtils::shareList(const ListView& list)
{
    qDebug() << "Share list:" << list.getName();
    shareListUri(list.getUri());
}

void ShareUtils::shareListUri(const QString& listUri)
{
    ATProto::ATUri atUri(listUri);

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());
    share(shareUri, "List link");
}

void ShareUtils::shareStarterPack(const StarterPackViewBasic& starterPack)
{
    qDebug() << "Share starter pack:" << starterPack.getName();
    ATProto::ATUri atUri(starterPack.getUri());

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());
    share(shareUri, "Starter pack link");
}

void ShareUtils::shareAuthor(const BasicProfile& author)
{
    const QString& authorId = author.getDid();
    const QString shareUri = QString("https://bsky.app/profile/%1").arg(authorId);
    share(shareUri, "Author link");
}

void ShareUtils::shareConvo(const ConvoView& convo)
{
    const QString shareUri = convo.getInviteLinkUrl();
    share(shareUri, "Invite link");
}

void ShareUtils::shareSearch(const QString& searchText, const SearchOptions& searchOptions)
{
    const QString shareUri = LinkUtils::createSearchLink(searchText, searchOptions, mSkywalker->getUser().getHandle());
    share(shareUri, "Search link");
}

void ShareUtils::copyAuthorLinkToClipboard(const BasicProfile& author)
{
    const QString& authorId = author.getDid();
    const QString shareUri = QString("https://bsky.app/profile/%1").arg(authorId);
    copyToClipboard(shareUri);
}

void ShareUtils::openLinkInApp(const QString& link)
{
#ifdef Q_OS_ANDROID
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject jLink = QJniObject::fromString(link);

    activity.callMethod<void>(
        "openLinkInApp",
        "(Ljava/lang/String;)V",
        jLink.object<jstring>());
#else
    qWarning() << "Cannot open link in app:" << link;
#endif
}

void ShareUtils::copyPostTextToClipboard(const QString& text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    mSkywalker->showStatusMessage(tr("Post text copied to clipboard"), QEnums::STATUS_LEVEL_INFO);
}

void ShareUtils::copyToClipboard(const QString& text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    mSkywalker->showStatusMessage(tr("Copied to clipboard"), QEnums::STATUS_LEVEL_INFO);
}

void ShareUtils::share(const QString& uri, const QString& subject)
{
#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(uri);
    QJniObject jSubject = QJniObject::fromString(subject);

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(uri);
    mSkywalker->showStatusMessage(tr("%1 copied to clipboard").arg(subject), QEnums::STATUS_LEVEL_INFO);
#endif
}

}
