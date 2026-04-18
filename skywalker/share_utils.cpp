// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "share_utils.h"
#include "skywalker.h"
#include <QClipboard>

namespace Skywalker {

ShareUtils::ShareUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void ShareUtils::sharePost(const QString& postUri)
{
    qDebug() << "Share post:" << postUri;
    ATProto::ATUri atUri(postUri);

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("post");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    mSkywalker->showStatusMessage(tr("Post link copied to clipboard"), QEnums::STATUS_LEVEL_INFO);
#endif
}

void ShareUtils::shareFeed(const GeneratorView& feed)
{
    qDebug() << "Share feed:" << feed.getDisplayName();
    ATProto::ATUri atUri(feed.getUri());

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("feed");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    mSkywalker->showStatusMessage(tr("Feed link copied to clipboard"), QEnums::STATUS_LEVEL_INFO);
#endif
}


void ShareUtils::shareList(const ListView& list)
{
    qDebug() << "Share list:" << list.getName();
    ATProto::ATUri atUri(list.getUri());

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("list");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    mSkywalker->showStatusMessage(tr("List link copied to clipboard"), QEnums::STATUS_LEVEL_INFO);
#endif
}

void ShareUtils::shareStarterPack(const StarterPackViewBasic& starterPack)
{
    qDebug() << "Share starter pack:" << starterPack.getName();
    ATProto::ATUri atUri(starterPack.getUri());

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("starter pack");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    mSkywalker->showStatusMessage(tr("Starter pack link copied to clipboard"), QEnums::STATUS_LEVEL_INFO);
#endif
}

void ShareUtils::shareAuthor(const BasicProfile& author)
{
    const QString& authorId = author.getDid();
    const QString shareUri = QString("https://bsky.app/profile/%1").arg(authorId);

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("author profile");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    mSkywalker->showStatusMessage(tr("Author link copied to clipboard"), QEnums::STATUS_LEVEL_INFO);
#endif
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

}
