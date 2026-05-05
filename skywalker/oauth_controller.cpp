// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "oauth_controller.h"
#ifdef Q_OS_ANDROID
#include "jni_callback.h"
#else
#include "file_utils.h"
#endif
#include <atproto/lib/oauth.h>
#include <QTcpServer>

namespace Skywalker {

static constexpr char const* CLIENT_ID = "https://thereforeiam.eu/skywalker/oauth/client-metadata.json";

const QStringList SCOPE = {
    ATProto::OAuth::SCOPE_ATPROTO,
    "account:email?action=read",

    "repo:app.bsky.actor.profile",
    "repo:app.bsky.actor.status",
    "repo:app.bsky.feed.like",
    "repo:app.bsky.feed.post",
    "repo:app.bsky.feed.postgate",
    "repo:app.bsky.feed.repost",
    "repo:app.bsky.feed.threadgate",
    "repo:app.bsky.graph.block",
    "repo:app.bsky.graph.follow",
    "repo:app.bsky.graph.list",
    "repo:app.bsky.graph.listblock",
    "repo:app.bsky.graph.listitem",
    "repo:app.bsky.graph.starterpack",
    "repo:app.bsky.notification.declaration",
    "repo:chat.bsky.actor.declaration",

    "rpc:app.bsky.actor.getPreferences?aud=*",
    "rpc:app.bsky.actor.getProfile?aud=*",
    "rpc:app.bsky.actor.getProfiles?aud=*",
    "rpc:app.bsky.actor.getSuggestions?aud=*",
    "rpc:app.bsky.actor.putPreferences?aud=*",
    "rpc:app.bsky.actor.searchActors?aud=*",
    "rpc:app.bsky.actor.searchActorsTypeahead?aud=*",
    "rpc:app.bsky.bookmark.createBookmark?aud=*",
    "rpc:app.bsky.bookmark.deleteBookmark?aud=*",
    "rpc:app.bsky.bookmark.getBookmarks?aud=*",
    "rpc:app.bsky.feed.getActorFeeds?aud=*",
    "rpc:app.bsky.feed.getActorLikes?aud=*",
    "rpc:app.bsky.feed.getAuthorFeed?aud=*",
    "rpc:app.bsky.feed.getFeed?aud=*",
    "rpc:app.bsky.feed.getFeedGenerator?aud=*",
    "rpc:app.bsky.feed.getFeedGenerators?aud=*",
    "rpc:app.bsky.feed.getLikes?aud=*",
    "rpc:app.bsky.feed.getListFeed?aud=*",
    "rpc:app.bsky.feed.getPostThread?aud=*",
    "rpc:app.bsky.feed.getPosts?aud=*",
    "rpc:app.bsky.feed.getQuotes?aud=*",
    "rpc:app.bsky.feed.getRepostedBy?aud=*",
    "rpc:app.bsky.feed.getSuggestedFeeds?aud=*",
    "rpc:app.bsky.feed.getTimeline?aud=*",
    "rpc:app.bsky.feed.searchPosts?aud=*",
    "rpc:app.bsky.feed.sendInteractions?aud=*",
    "rpc:app.bsky.graph.getActorStarterPacks?aud=*",
    "rpc:app.bsky.graph.getBlocks?aud=*",
    "rpc:app.bsky.graph.getFollowers?aud=*",
    "rpc:app.bsky.graph.getFollows?aud=*",
    "rpc:app.bsky.graph.getKnownFollowers?aud=*",
    "rpc:app.bsky.graph.getList?aud=*",
    "rpc:app.bsky.graph.getListBlocks?aud=*",
    "rpc:app.bsky.graph.getListMutes?aud=*",
    "rpc:app.bsky.graph.getLists?aud=*",
    "rpc:app.bsky.graph.getListsWithMembership?aud=*",
    "rpc:app.bsky.graph.getMutes?aud=*",
    "rpc:app.bsky.graph.getStarterPack?aud=*",
    "rpc:app.bsky.graph.getStarterPacks?aud=*",
    "rpc:app.bsky.graph.getStarterPacksWithMembership?aud=*",
    "rpc:app.bsky.graph.getSuggestedFollowsByActor?aud=*",
    "rpc:app.bsky.graph.muteActor?aud=*",
    "rpc:app.bsky.graph.muteActorList?aud=*",
    "rpc:app.bsky.graph.muteThread?aud=*",
    "rpc:app.bsky.graph.unmuteActor?aud=*",
    "rpc:app.bsky.graph.unmuteActorList?aud=*",
    "rpc:app.bsky.graph.unmuteThread?aud=*",
    "rpc:app.bsky.labeler.getServices?aud=*",
    "rpc:app.bsky.notification.getPreferences?aud=*",
    "rpc:app.bsky.notification.getUnreadCount?aud=*",
    "rpc:app.bsky.notification.listActivitySubscriptions?aud=*",
    "rpc:app.bsky.notification.listNotifications?aud=*",
    "rpc:app.bsky.notification.putActivitySubscription?aud=*",
    "rpc:app.bsky.notification.putPreferences?aud=*",
    "rpc:app.bsky.notification.putPreferencesV2?aud=*",
    "rpc:app.bsky.notification.updateSeen?aud=*",
    "rpc:app.bsky.unspecced.getPopularFeedGenerators?aud=*",
    "rpc:app.bsky.unspecced.getSuggestedStarterPacks?aud=*",
    "rpc:app.bsky.unspecced.getSuggestedUsers?aud=*",
    "rpc:app.bsky.unspecced.getTrends?aud=*",
    "rpc:app.bsky.video.getJobStatus?aud=*",
    "rpc:app.bsky.video.getUploadLimits?aud=*",
    "rpc:app.bsky.video.uploadVideo?aud=*",

    "rpc:chat.bsky.convo.acceptConvo?aud=*",
    "rpc:chat.bsky.convo.addReaction?aud=*",
    "rpc:chat.bsky.convo.deleteMessageForSelf?aud=*",
    "rpc:chat.bsky.convo.getConvo?aud=*",
    "rpc:chat.bsky.convo.getConvoForMembers?aud=*",
    "rpc:chat.bsky.convo.getMessages?aud=*",
    "rpc:chat.bsky.convo.leaveConvo?aud=*",
    "rpc:chat.bsky.convo.listConvos?aud=*",
    "rpc:chat.bsky.convo.muteConvo?aud=*",
    "rpc:chat.bsky.convo.removeReaction?aud=*",
    "rpc:chat.bsky.convo.sendMessage?aud=*",
    "rpc:chat.bsky.convo.unmuteConvo?aud=*",
    "rpc:chat.bsky.convo.updateRead?aud=*"
};

QString OAuthController::getClientId()
{
    return CLIENT_ID;
}

QStringList OAuthController::getScope()
{
    return SCOPE;
}

#ifdef DEBUG
QString OAuthController::getTestClientId()
{
    return "https://thereforeiam.eu/skywalker/oauth/test-client-metadata.json";
}

QStringList OAuthController::getTestScope()
{
    return SCOPE;
}
#endif

#ifndef Q_OS_ANDROID
QString OAuthController::getKeyStorageFilename(const QString& did)
{
    return QString("%1/dpop.pem").arg(FileUtils::getAppDataPath(did));
}

QString OAuthController::getTestPassPhrase()
{
    return "Rage! Rage! Against the dying of the light.";
}
#endif

bool OAuthController::start(const RedirectCb& redirectCb)
{
#ifndef Q_OS_ANDROID
    qDebug() << "Start HTTP server";
    auto* tcpServer = new QTcpServer();

    mHttpServer.route("/oauth/callback",
        [redirectCb](const QHttpServerRequest& request, QHttpServerResponder& responder){
            qDebug() << "oauth callback:" << request.url();
            redirectCb(request.url());
            responder.write(QHttpServerResponder::StatusCode::NoContent);
        });

    if (!tcpServer->listen(QHostAddress::LocalHost, LISTEN_PORT) || !mHttpServer.bind(tcpServer))
    {
        qWarning() << "Failed to listen on port:" << LISTEN_PORT;
        delete tcpServer;
        return false;
    }

    qDebug() << "Listening on port:" << tcpServer->serverPort();
    return true;
#else
    // Intent handling may not have started yet. Make sure it is.
    JNICallbackListener::handlePendingIntent();
    mRedirectCb = redirectCb;
    return true;
#endif
}

void OAuthController::redirect(const QString& url)
{
    qDebug() << "Redirect:" << url;

    if (!mRedirectCb)
    {
        qWarning() << "Redirect call back not set";
        return;
    }

    mRedirectCb(url);
}

}
