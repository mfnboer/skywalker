// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "link_card_reader.h"
#include "definitions.h"
#include "skywalker.h"
#include "unicode_fonts.h"
#include <QRegularExpression>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QUrlQuery>

namespace Skywalker {

// Store cookies for this session only. No permanent storage!
class CookieJar : public QNetworkCookieJar
{
public:
    QList<QNetworkCookie> cookiesForUrl(const QUrl& url) const override
    {
        qDebug() << "Get cookies for:" << url;
        return QNetworkCookieJar::cookiesForUrl(url);
    }

    bool setCookiesFromUrl(const QList<QNetworkCookie>& cookieList, const QUrl& url) override
    {
        qDebug() << "Set cookies from:" << url;

        for (const auto& cookie : cookieList)
            qDebug() << "Cookie:" << cookie.name() << "=" << cookie.value() << "domain:" << cookie.domain() << "path:" << cookie.path();

        bool retval = QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
        qDebug() << "Cookies accepted:" << retval;
        return retval;
    }

    bool setCookiesFromReply(const QNetworkReply& reply)
    {
        const QVariant setCookie = reply.header(QNetworkRequest::SetCookieHeader);
        if (setCookie.isValid())
        {
            const auto cookies = setCookie.value<QList<QNetworkCookie>>();
            return setCookiesFromUrl(cookies, reply.url());
        }

        return false;
    }
};

LinkCardReader::LinkCardReader(QObject* parent):
    QObject(parent),
    mCardCache(100),
    mGifUtils(this)
{
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(10000);
    mNetwork.setCookieJar(new CookieJar);

    QLocale locale;
    mAcceptLanguage = QString("%1_%2, *;q=0.5").arg(
        QLocale::languageToCode(locale.language()),
        QLocale::territoryToCode(locale.territory()));
    qDebug() << "Accept-Language:" << mAcceptLanguage;
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

void LinkCardReader::getLinkCard(const QString& link, bool retry)
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
        emit linkCardFailed();
        return;
    }

    auto* card = mCardCache[url];
    if (card)
    {
        qDebug() << "Got card from cache:" << card->getLink();

        if (!card->isEmpty())
        {
            emit linkCard(card);
        }
        else
        {
            qDebug() << "Card is empty";
            emit linkCardFailed();
        }

        return;
    }

    if (mGifUtils.isGiphyLink(url.toString()))
    {
        qDebug() << "Giphy URL:" << url;
        const QString gifUrl = mGifUtils.getGifUrl(url.toString());

        if (!gifUrl.isNull())
        {
            qDebug() << "Create Giphy link card";

            auto* card = makeLinkCard(
                gifUrl,
                QString(tr("Giphy GIF posted from Skywalker %1").arg(SKYWALKER_HANDLE)),
                QString(tr("This GIF has been posted from Skywalker for Android. Get Skywalker from Google Play.<br>Bluesky: %1")).arg(SKYWALKER_HANDLE),
                gifUrl);

            emit linkCard(card);
            return;
        }
    }

    QNetworkRequest request(url);

    // Without this YouTube Shorts does not load
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, true);

    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::UserVerifiedRedirectPolicy);
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Encoding", "identity");
    request.setRawHeader("Accept-Language", mAcceptLanguage.toUtf8()); // For Reuters
    request.setRawHeader("Priority", "i"); // For Reuters
    request.setRawHeader("User-Agent", Skywalker::getUserAgentString().toUtf8()); // For NYT, Reuters

    QNetworkReply* reply = mNetwork.get(request);
    mInProgress = reply;
    mPrevDestination = url;
    mRetry = retry;

    connect(reply, &QNetworkReply::finished, this, [this, reply]{ extractLinkCard(reply); });
    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](auto errCode){ requestFailed(reply, errCode); });
    connect(reply, &QNetworkReply::sslErrors, this, [this, reply]{ requestSslFailed(reply); });
    connect(reply, &QNetworkReply::redirected, this, [this, reply](const QUrl& url){ redirect(reply, url); });
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
    static const QString ogTitleStr1(R"(<meta [^>]*(property|name) *=[\"']?(og:|twitter:)?title[\"']? [^>]*content=%1(?<title>[^%1]+?)%1[^>]*>)");
    static const QString ogTitleStr2(R"(<meta [^>]*content=%1(?<title>[^%1]+?)%1 [^>]*(property|name)=[\"']?(og:|twitter:)?title[\"']?[^>]*>)");

    static const std::vector<QRegularExpression> ogTitleREs = {
        QRegularExpression(ogTitleStr1.arg('"')),
        QRegularExpression(ogTitleStr1.arg('\'')),
        QRegularExpression(ogTitleStr2.arg('"')),
        QRegularExpression(ogTitleStr2.arg('\''))
    };

    static const QString ogDescriptionStr1(R"(<meta [^>]*(property|name) *=[\"']?(og:|twitter:)?description[\"']? [^>]*content=%1(?<description>[^%1]+?)%1[^>]*>)");
    static const QString ogDescriptionStr2(R"(<meta [^>]*content=%1(?<description>[^%1]+?)%1 [^>]*(property|name)=[\"']?(og:|twitter:)?description[\"']?[^>]*>)");

    static const std::vector<QRegularExpression> ogDescriptionREs = {
        QRegularExpression(ogDescriptionStr1.arg('"')),
        QRegularExpression(ogDescriptionStr1.arg('\'')),
        QRegularExpression(ogDescriptionStr2.arg('"')),
        QRegularExpression(ogDescriptionStr2.arg('\''))
    };

    static const QString ogImageStr1(R"(<meta [^>]*(property|name) *=[\"']?(og:|twitter:)?image[\"']? [^>]*content=%1(?<image>[^%1]+?)%1[^>]*>)");
    static const QString ogImageStr2(R"(<meta [^>]*content=%1(?<image>[^%1]+?)%1 [^>]*(property|name)=[\"']?(og:|twitter:)?image[\"']?[^>]*>)");
    static const QString ogImageStr3(R"(<meta [^>]*(property|name) *=[\"']?(og:|twitter:)?image[\"']? [^>]*content=(?<image>[^ \"]+?)[^>]*>)");
    static const QString ogImageStr4(R"(<meta [^>]*content=(?<image>[^ \"]+?) [^<]*(property|name)=[\"']?(og:|twitter:)?image[\"']?[^>]*>)");

    static const std::vector<QRegularExpression> ogImageREs = {
        QRegularExpression(ogImageStr1.arg('"')),
        QRegularExpression(ogImageStr1.arg('\'')),
        QRegularExpression(ogImageStr2.arg('"')),
        QRegularExpression(ogImageStr2.arg('\'')),
        QRegularExpression(ogImageStr3),
        QRegularExpression(ogImageStr4)
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
        card->setTitle(toPlainText(title));

    const QString description = matchRegexes(ogDescriptionREs, data, "description");
    if (!description.isEmpty())
        card->setDescription(toPlainText(description));

    QString imgUrlString = matchRegexes(ogImageREs, data, "image");
    qDebug() << "img url:" << imgUrlString;
    const auto& url = reply->request().url();

    // Washington post wraps the IMG url in a PHP query like this:
    // https://www.washingtonpost.com/wp-apps/imrs.php?src=https://<img-url>&w=1440
    if (imgUrlString.contains(".php?src=http"))
    {
        const QUrl phpUrl(imgUrlString);
        const QString queryString = phpUrl.query();

        if (!queryString.isEmpty())
        {
            const QUrlQuery query(queryString);
            const QString src = query.queryItemValue("src");
            imgUrlString = src;
            qDebug() << "php scr img url:" << imgUrlString;
        }
        else
        {
            imgUrlString.clear();
        }
    }

    if (!imgUrlString.isEmpty())
    {
        // URL's from Instagram seem to be HTML encoded and contain &amp; instead of just &
        if (imgUrlString.contains("&amp;"))
        {
            imgUrlString = toPlainText(imgUrlString);
            qDebug() << "plain text img url:" << imgUrlString;
        }

        QUrl imgUrl(imgUrlString);

        if (imgUrl.isRelative())
        {
            if (QDir::isAbsolutePath(imgUrlString))
            {
                imgUrl.setHost(url.host());
                imgUrl.setScheme(url.scheme());
                imgUrl.setPort(url.port());
                card->setThumb(imgUrl.toString());
            }
            else
            {
                card->setThumb(url.toString() + imgUrlString);
            }

            qDebug() << "Relative img url:" << imgUrlString << "url:" << imgUrl << "valid:" << imgUrl.isValid() << "thumb:" << card->getThumb();
        }
        else
        {
            card->setThumb(imgUrlString);
            qDebug() << "Full img url:" << imgUrlString << "url:" << imgUrl << "valid:" << imgUrl.isValid();
        }
    }

    if (card->isEmpty())
    {
        auto* cookieJar = static_cast<CookieJar*>(mNetwork.cookieJar());
        Q_ASSERT(cookieJar);

        if (!mRetry && cookieJar->setCookiesFromReply(*reply))
        {
            qDebug() << "Cookies stored, retry";
            getLinkCard(url.toString(), true);
        }
        else
        {
            mCardCache.insert(url, card.release());
            qDebug() << url << "has no link card.";
            emit linkCardFailed();
        }

        return;
    }

    card->setLink(url.toString());
    mCardCache.insert(url, card.get());
    emit linkCard(card.release());
}

QString LinkCardReader::toPlainText(const QString& text)
{
    // Texts in linkcard text often contain double encoded ampersands, e.g.
    // &amp;#8216;
    // That should be &#8216;
    QString plain(text);
    plain.replace("&amp;#", "&#");
    return UnicodeFonts::toPlainText(plain);
}

void LinkCardReader::requestFailed(QNetworkReply* reply, int errCode)
{
    mInProgress = nullptr;
    qDebug() << "Failed to get link:" << reply->request().url();
    qDebug() << "Error:" << errCode << reply->errorString();
    qDebug() << reply->readAll();
    emit linkCardFailed();
}

void LinkCardReader::requestSslFailed(QNetworkReply* reply)
{
    mInProgress = nullptr;
    qDebug() << "SSL error, failed to get link:" << reply->request().url();
    emit linkCardFailed();
}

void LinkCardReader::redirect(QNetworkReply* reply, const QUrl& redirectUrl)
{
    qDebug() << "Prev url:" << mPrevDestination << "redirect url:" << redirectUrl;

    // Allow: https -> https, http -> http, http -> https
    // Allow https -> http only if the host stays the same
    if (mPrevDestination.scheme() == redirectUrl.scheme() ||
        mPrevDestination.scheme() == "http" ||
        mPrevDestination.host() == redirectUrl.host())
    {
        emit reply->redirectAllowed();
    }

    auto* cookieJar = static_cast<CookieJar*>(mNetwork.cookieJar());
    Q_ASSERT(cookieJar);
    cookieJar->setCookiesFromReply(*reply);

    mPrevDestination = redirectUrl;
}

}
