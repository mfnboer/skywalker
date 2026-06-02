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

signals:
    void linkCard(LinkCard*);
    void linkCardRichEmbedFailed(QString msg);
    void linkCardFailed();
    void linkGif(TenorGif);

private:
    QString toPlainText(const QString& text);
    void extractLinkCard(QNetworkReply* reply);
    void getEmbedExternalView(LinkCard* card, const std::vector<QString> associatedUris);
    void requestFailed(QNetworkReply* reply, int errCode);
    void requestSslFailed(QNetworkReply* reply);
    void redirect(QNetworkReply* reply, const QUrl& redirectUrl);

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
