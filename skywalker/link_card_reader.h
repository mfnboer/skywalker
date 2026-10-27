// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "link_card.h"
#include "gif_utils.h"
#include "presence.h"
#include "profile.h"
#include "tenor_gif.h"
#include "wrapped_skywalker.h"

#include <QCache>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtQmlIntegration>

namespace Skywalker {

class LinkCardReader : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool inProgress READ getInProgress NOTIFY inProgressChanged FINAL)
    QML_ELEMENT

public:
    explicit LinkCardReader(QObject* parent = nullptr);

    bool getInProgress() const { return mInProgress != nullptr; }
    void setInProgress(QNetworkReply* reply);

    Q_INVOKABLE void getLinkCard(const QString& link, bool retry = false, bool cookieSaveControl = false);
    Q_INVOKABLE LinkCard* makeLinkCard(const QString& link, const QString& title,
                                  const QString& description, const QString& thumb);
    Q_INVOKABLE void getFavIcon(const QString& link, bool retry = false, bool cookieSaveControl = false);
    Q_INVOKABLE static ProfileLinkView makeProfileLinkView(const QString& uri, const QString& cid, const QString& url, const QString& title, const QString& icon);

signals:
    void linkCard(LinkCard*);
    void linkCardRichEmbedFailed(QString msg);
    void linkCardFailed();
    void linkGif(TenorGif);
    void favIcon(ProfileLinkView link);
    void favIconFailed();
    void inProgressChanged();

private:
    QNetworkReply* sendRequest(const QUrl& url, bool retry, bool cookieSaveControl);
    QString toPlainText(const QString& text);

    using GetFun = std::function<void(QString url, bool retry, bool cookieSaveControl)>;
    bool retryHack(const QUrl& requestUrl, const QString& title, const GetFun& getFun);
    bool retryCookie(QNetworkReply* reply, const GetFun& getFun);

    void extractLinkCard(QNetworkReply* reply);
    void getEmbedExternalView(LinkCard* card, const std::vector<QString> associatedUris);
    void linkCardRequestFailed(QNetworkReply* reply, int errCode);
    void linkCardRequestSslFailed(QNetworkReply* reply);
    void redirect(QNetworkReply* reply, const QUrl& redirectUrl);
    void getJoinRequestLinkCard(const QUrl& url);

    void extractFavIcon(const QString& link, QNetworkReply* reply);
    QString getIconUrl(const QByteArray& data);
    QString selectBestIconUrl(const QStringList& iconLinks);
    void favIconRequestFailed(QNetworkReply* reply, int errCode);
    void favIconRequestSslFailed(QNetworkReply* reply);

    QNetworkAccessManager* mNetwork;
    QCache<QUrl, LinkCard> mCardCache;
    QNetworkReply* mInProgress = nullptr;
    bool mEmbedExternalViewInProgress = false;
    QUrl mPrevDestination;
    bool mRetry = false;
    bool mCookieSaveControl = false;
    GifUtils mGifUtils;
    QString mAcceptLanguage;
};

}
