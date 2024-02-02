// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "link_card_reader.h"
#include "unicode_fonts.h"
#include <QRegularExpression>

namespace Skywalker {

LinkCardReader::LinkCardReader(QObject* parent):
    QObject(parent),
    mCardCache(100)
{
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(15000);
}

LinkCard* LinkCardReader::makeLinkCard(const QString& link, const QString& title,
                       const QString& description, const QString& thumb)
{
    LinkCard* card = new LinkCard(this);
    card->setLink(link);
    card->setTitle(title);
    card->setDescription(description);
    card->setThumb(thumb);

    QUrl url(link);
    mCardCache.insert(url, card);
    return mCardCache[url];
}

void LinkCardReader::getLinkCard(const QString& link)
{
    qDebug() << "Get link card:" << link;

    if (mInProgress)
    {
        mInProgress->abort();
        mInProgress = nullptr;
    }

    QString cleanedLink = link.startsWith("http") ? link : "https://" + link;
    if (cleanedLink.endsWith("/"))
        cleanedLink = cleanedLink.sliced(0, cleanedLink.size() - 1);

    QUrl url(cleanedLink);
    if (!url.isValid())
    {
        qWarning() << "Invalid link:" << link;
        return;
    }

    auto* card = mCardCache[url];
    if (card)
    {
        qDebug() << "Got card from cache:" << card->getLink();

        if (!card->isEmpty())
            emit linkCard(card);
        else
            qDebug() << "Card is empty";

        return;
    }

    QNetworkRequest request(url);
    QNetworkReply* reply = mNetwork.get(request);
    mInProgress = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply]{ extractLinkCard(reply); });
    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](auto errCode){ requestFailed(reply, errCode); });
    connect(reply, &QNetworkReply::sslErrors, this, [this, reply]{ requestSslFailed(reply); });
}

static QString matchRegexes(const std::vector<QRegularExpression>& regexes, const QByteArray& data, const QString& group)
{
    for (const auto& re : regexes)
    {
        auto match = re.match(data);

        if (match.hasMatch())
            return match.captured(group);
    }

    return {};
}

void LinkCardReader::extractLinkCard(QNetworkReply* reply)
{
    static const QString ogTitleStr1(R"(<meta [^>]*(property|name) *=[\"'](og:|twitter:)?title[\"'] [^>]*content=%1(?<title>[^%1]+?)%1[^>]*>)");
    static const QString ogTitleStr2(R"(<meta [^>]*content=%1(?<title>[^%1]+?)%1 [^>]*(property|name)=[\"'](og:|twitter:)?title[\"'][^>]*>)");

    static const std::vector<QRegularExpression> ogTitleREs = {
        QRegularExpression(ogTitleStr1.arg('"')),
        QRegularExpression(ogTitleStr1.arg('\'')),
        QRegularExpression(ogTitleStr2.arg('"')),
        QRegularExpression(ogTitleStr2.arg('\''))
    };

    static const QString ogDescriptionStr1(R"(<meta [^>]*(property|name) *=[\"'](og:|twitter:)?description[\"'] [^>]*content=%1(?<description>[^%1]+?)%1[^>]*>)");
    static const QString ogDescriptionStr2(R"(<meta [^>]*content=%1(?<description>[^%1]+?)%1 [^>]*(property|name)=[\"'](og:|twitter:)?description[\"'][^>]*>)");

    static const std::vector<QRegularExpression> ogDescriptionREs = {
        QRegularExpression(ogDescriptionStr1.arg('"')),
        QRegularExpression(ogDescriptionStr1.arg('\'')),
        QRegularExpression(ogDescriptionStr2.arg('"')),
        QRegularExpression(ogDescriptionStr2.arg('\''))
    };

    static const QString ogImageStr1(R"(<meta [^>]*(property|name) *=[\"'](og:|twitter:)?image[\"'] [^>]*content=%1(?<image>[^%1]+?)%1[^>]*>)");
    static const QString ogImageStr2(R"(<meta [^>]*content=%1(?<image>[^%1]+?)%1 [^>]*(property|name)=[\"'](og:|twitter:)?image[\"'][^>]*>)");

    static const std::vector<QRegularExpression> ogImageREs = {
        QRegularExpression(ogImageStr1.arg('"')),
        QRegularExpression(ogImageStr1.arg('\'')),
        QRegularExpression(ogImageStr2.arg('"')),
        QRegularExpression(ogImageStr2.arg('\''))
    };

    mInProgress = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        requestFailed(reply, reply->error());
        return;
    }

    auto card = std::make_unique<LinkCard>(this);
    const auto data = reply->readAll();

    const QString title = matchRegexes(ogTitleREs, data, "title");
    if (!title.isEmpty())
        card->setTitle(UnicodeFonts::toPlainText(title));

    const QString description = matchRegexes(ogDescriptionREs, data, "description");
    if (!description.isEmpty())
        card->setDescription(UnicodeFonts::toPlainText(description));

    const QString imgUrl = matchRegexes(ogImageREs, data, "image");;
    const auto& url = reply->request().url();

    if (!imgUrl.isEmpty())
    {
        if (imgUrl.startsWith("/"))
            card->setThumb(url.toString() + imgUrl);
        else
            card->setThumb(imgUrl);
    }

    if (card->isEmpty())
    {
        mCardCache.insert(url, card.release());
        qDebug() << url << "has no link card.";
        return;
    }

    card->setLink(url.toString());
    mCardCache.insert(url, card.get());
    emit linkCard(card.release());
}

void LinkCardReader::requestFailed(QNetworkReply* reply, int errCode)
{
    mInProgress = nullptr;
    qDebug() << "Failed to get link:" << reply->request().url();
    qDebug() << "Error:" << errCode << reply->errorString();
}

void LinkCardReader::requestSslFailed(QNetworkReply* reply)
{
    mInProgress = nullptr;
    qDebug() << "SSL error, failed to get link:" << reply->request().url();
}

}
