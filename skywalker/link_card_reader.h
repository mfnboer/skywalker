// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "link_card.h"
#include "gif_utils.h"
#include "presence.h"
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
    QML_ELEMENT

public:
    explicit LinkCardReader(QObject* parent = nullptr);

    Q_INVOKABLE void getLinkCard(const QString& link, bool retry = false, bool cookieSaveControl = false);
    Q_INVOKABLE LinkCard* makeLinkCard(const QString& link, const QString& title,
                                  const QString& description, const QString& thumb);
    Q_INVOKABLE void getFavIcon(const QString& link, bool retry = false, bool cookieSaveControl = false);

signals:
    void linkCard(LinkCard*);
    void linkCardRichEmbedFailed(QString msg);
    void linkCardFailed();
    void linkGif(TenorGif);
    void favIcon(QString title, QString iconUrl);
    void favIconFailed();

private:
    QNetworkReply* sendRequest(const QUrl& url, bool retry, bool cookieSaveControl);
    QString toPlainText(const QString& text);
    void extractLinkCard(QNetworkReply* reply);
    void getEmbedExternalView(LinkCard* card, const std::vector<QString> associatedUris);
    void linkCardRequestFailed(QNetworkReply* reply, int errCode);
    void linkCardRequestSslFailed(QNetworkReply* reply);
    void redirect(QNetworkReply* reply, const QUrl& redirectUrl);
    void getJoinRequestLinkCard(const QUrl& url);

    void extractFavIcon(QNetworkReply* reply);
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
