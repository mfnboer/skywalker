// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "post_feed_replay.h"
#include "file_utils.h"
#include "post_feed_model.h"
#include "lexicon/lexicon.h"
#include <atproto/lib/xjson.h>
#include <QCborValue>
#include <QCborStreamWriter>

namespace Skywalker {

std::unique_ptr<QThread> PostFeedReplay::sLoadThread;
PostFeedReplay::SharedPtr PostFeedReplay::sLoadedReplay;
PostFeedReplay::LoadCb PostFeedReplay::sLoadCb;

void PostFeedReplay::clear()
{
    qDebug() << "Clear";
    mPagePrependSequence.clear();
    mDirty = true;
}

void PostFeedReplay::append(ATProto::AppBskyFeed::OutputFeed::SharedPtr page)
{
    qDebug() << "Append page, size:" << page->mFeed.size() << "cursor:" << page->mCursor.value_or("");

    if (mPagePrependSequence.empty())
        mPagePrependSequence.push_back(std::make_shared<PageList>());

    auto& pageList = mPagePrependSequence.front();
    pageList->mPages.push_back(page);
    mDirty = true;
}

void PostFeedReplay::prepend(ATProto::AppBskyFeed::OutputFeed::SharedPtr page, bool gapTillNextPage)
{
    qDebug() << "Prepend page, size:" << page->mFeed.size() << "cursor:" << page->mCursor.value_or("") << "gapTillNextPage:" << gapTillNextPage;
    auto pageList = std::make_shared<PageList>();
    pageList->mPages.push_back(page);
    pageList->mGapTillNextPage = gapTillNextPage;
    mPagePrependSequence.push_back(pageList);
    mDirty = true;
}

void PostFeedReplay::gapFill(ATProto::AppBskyFeed::OutputFeed::SharedPtr page, const QString& cursor, bool gapTillNextPage)
{
    qDebug() << "Gap fill, size:" << page->mFeed.size() << "cursor:" << page->mCursor.value_or("") << "gapCursor:" << cursor << "gapTillNextPage:" << gapTillNextPage;
    auto* pageList = findGap(cursor);

    Q_ASSERT(pageList);
    if (!pageList)
    {
        qWarning() << "Cannot find gap:" << cursor;
        return;
    }

    Q_ASSERT(pageList->mGapTillNextPage);
    pageList->mPages.push_back(page);
    pageList->mGapTillNextPage = gapTillNextPage;
    mDirty = true;
}

void PostFeedReplay::closeGap(const QString& cursor)
{
    qDebug() << "Close gap, gapCursor:" << cursor;
    auto* pageList = findGap(cursor);

    Q_ASSERT(pageList);
    if (!pageList)
    {
        qWarning() << "Cannot find gap:" << cursor;
        return;
    }

    Q_ASSERT(pageList->mGapTillNextPage);
    pageList->mGapTillNextPage = false;
    mDirty = true;
}

void PostFeedReplay::removeTail(const QString& cursor)
{
    qDebug() << "Remove tail, cursor:" << cursor;
    int i = 0;

    for (; i < (int)mPagePrependSequence.size(); ++i)
    {
        auto& pageList = mPagePrependSequence[i];
        int j = 0;

        for (; j < (int)pageList->mPages.size(); ++j)
        {
            auto& page = pageList->mPages[j];

            if (page->mCursor && *page->mCursor == cursor)
                break;
        }

        if (j < (int)pageList->mPages.size())
        {
            const int newSize = j + 1;
            pageList->mPages.resize(newSize);
            pageList->mGapTillNextPage = false;
            break;
        }
    }

    if (i < (int)mPagePrependSequence.size())
        mPagePrependSequence.erase(mPagePrependSequence.begin(), mPagePrependSequence.begin() + i);
    else
        qWarning() << "Cursor not found:" << cursor;

    mDirty = true;
}

void PostFeedReplay::removeHead(const QString& cursor)
{
    qDebug() << "Remove head, cursor:" << cursor;
    int i = 0;

    for (; i < (int)mPagePrependSequence.size(); ++i)
    {
        auto& pageList = mPagePrependSequence[i];
        int j = 0;

        for (; j < (int)pageList->mPages.size(); ++j)
        {
            auto& page = pageList->mPages[j];

            if (page->mCursor && *page->mCursor == cursor)
                break;
        }

        if (j < (int)pageList->mPages.size())
        {
            pageList->mPages.erase(pageList->mPages.begin(), pageList->mPages.begin() + j);
            break;
        }
    }

    if (i < (int)mPagePrependSequence.size())
        mPagePrependSequence.erase(mPagePrependSequence.begin() + i + 1, mPagePrependSequence.end());
    else
        qWarning() << "Cursor not found:" << cursor;

    mDirty = true;
}

void PostFeedReplay::replay(PostFeedModel& model) const
{
    qDebug() << "Replay:" << model.getFeedName();
    model.clear();

    if (mPagePrependSequence.empty())
    {
        qDebug() << "Nothing to replay";
        return;
    }

    const auto& firstPageList = mPagePrependSequence.front();

    for (const auto& page : firstPageList->mPages)
    {
        auto feed = page;
        model.addFeed(std::move(feed));
    }

    for (int i = 1; i < (int)mPagePrependSequence.size(); ++i)
    {
        const auto& pageList = mPagePrependSequence[i];

        if (pageList->mPages.empty())
        {
            qWarning() << "Empty page list:" << i;
            continue;
        }

        auto feed = pageList->mPages.front();
        int gapId = model.prependFeed(std::move(feed));

        for (int j = 1; j < (int)pageList->mPages.size(); ++j)
        {
            if (gapId == 0)
            {
                qWarning() << "No gap to fill:" << j;
                break;
            }

            feed = pageList->mPages[j];
            gapId = model.gapFillFeed(std::move(feed), gapId);
        }

        if (gapId > 0 && !pageList->mGapTillNextPage)
        {
            // Close gap with empty feed page
            feed = std::make_shared<ATProto::AppBskyFeed::OutputFeed>();
            model.gapFillFeed(std::move(feed), gapId);
        }
    }

    qDebug() << "Replay done:" << model.getFeedName();
}

PostFeedReplay::PageList* PostFeedReplay::findGap(const QString& cursor)
{
    for (auto& pageList : mPagePrependSequence)
    {
        Q_ASSERT(!pageList->mPages.empty());
        auto& lastPage = pageList->mPages.back();

        if (lastPage->mCursor && *lastPage->mCursor == cursor)
            return pageList.get();
    }

    qDebug() << "Gap not found:" << cursor;
    return nullptr;
}

QJsonObject PostFeedReplay::toJson() const
{
    QJsonObject json;
    json.insert("$type", Lexicon::COLLECTION_POST_FEED_REPLAY);
    json.insert("pagePrependSequence", ATProto::XJsonObject::toJsonArray<PageList>(mPagePrependSequence));
    return json;
}

PostFeedReplay::SharedPtr PostFeedReplay::fromJson(const QJsonObject& json)
{
    auto replay = std::make_shared<PostFeedReplay>();
    ATProto::XJsonObject xjson(json);
    replay->mPagePrependSequence = xjson.getRequiredVector<PageList>("pagePrependSequence");
    return replay;
}

QJsonObject PostFeedReplay::PageList::toJson() const
{
    QJsonObject json;
    json.insert("$type", Lexicon::POST_FEED_REPLAY_DEFS_PAGE_LIST);
    QJsonArray jsonArray;

    for (const auto& page : mPages)
    {
        if (!page->mJson.isEmpty())
            jsonArray.append(page->mJson);
        else
            jsonArray.append(page->toJson());
    }

    json.insert("pages", jsonArray);
    ATProto::XJsonObject::insertOptionalJsonValue(json, "gapTillNextPage", mGapTillNextPage, false);
    return json;
}

PostFeedReplay::PageList::SharedPtr PostFeedReplay::PageList::fromJson(const QJsonObject& json)
{
    auto pageList = std::make_shared<PageList>();
    ATProto::XJsonObject xjson(json);
    pageList->mPages = xjson.getRequiredVector<ATProto::AppBskyFeed::OutputFeed>("pages");
    pageList->mGapTillNextPage = xjson.getOptionalBool("gapTillNextPage", false);
    return pageList;
}

QString PostFeedReplay::getReplayFileName(const QString& userDid, const QString& feedName)
{
    const QString path = FileUtils::getAppDataPath(userDid); // TODO cache data?

    if (path.isEmpty())
    {
        qWarning() << "Failed to get path:" << userDid;
        return {};
    }

    const QString fileName = QString("%1/replay_%2.cbor").arg(path, feedName);
    return fileName;
}

void PostFeedReplay::save(const QString& userDid, const QString& feedName)
{
    if (!mDirty)
        return;

    if (mSaveThread)
    {
        qWarning() << "Previous save still busy";
        return;
    }

    qDebug() << "To json";
    const auto json = toJson();

    qDebug() << "Start thread";
    QThread* thread = QThread::create([this, userDid, feedName, json]{
        save(userDid, feedName, json);
    });

    mSaveThread.reset(thread);
    connect(thread, &QThread::finished, this, [this]{ finishSave(); }, Qt::SingleShotConnection);
    mSaveThread->start();
    mDirty = false;
    qDebug() << "Thread started";
}

void PostFeedReplay::save(const QString& userDid, const QString& feedName, const QJsonObject json)
{
    qDebug() << "Save json:" << userDid << feedName;

    const QString fileName = getReplayFileName(userDid, feedName);

    if (fileName.isEmpty())
        return;

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot create file:" << fileName;
        file.remove();
        return;
    }

    QCborStreamWriter writer(&file);
    QCborValue::fromJsonValue(json).toCbor(writer);

    file.close();
    qDebug() << "Saved:" << fileName;
}

void PostFeedReplay::finishSave()
{
    mSaveThread->wait();
    mSaveThread = nullptr;
}

bool PostFeedReplay::load(const QString& userDid, const QString& feedName, const LoadCb& loadCb)
{
    if (sLoadThread)
    {
        qWarning() << "Previous load still busy";
        return false;
    }

    sLoadCb = loadCb;

    qDebug() << "Start thread";
    QThread* thread = QThread::create([userDid, feedName]{
        sLoadedReplay = loadContinue(userDid, feedName);
    });

    sLoadThread.reset(thread);
    static QObject* LOAD_CONTEXT = new QObject();
    connect(thread, &QThread::finished, LOAD_CONTEXT, []{ finishLoad(); }, Qt::SingleShotConnection);
    sLoadThread->start();
    qDebug() << "Thread started";
    return true;
}

PostFeedReplay::SharedPtr PostFeedReplay::loadContinue(const QString& userDid, const QString& feedName)
{
    qDebug() << "Load:" << userDid << feedName;
    const QString fileName = getReplayFileName(userDid, feedName);

    if (fileName.isEmpty())
        return nullptr;

    QFile file(fileName);

    if (!file.exists())
    {
        qDebug() << "File not available:" << fileName;
        return nullptr;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file:" << fileName;
        return nullptr;
    }

    QCborStreamReader reader(&file);

    if (reader.isTag() && reader.toTag() == QCborKnownTags::Signature)
        reader.next();

    QCborValue cbor = QCborValue::fromCbor(reader);
    const auto cborError = reader.lastError();

    if (cborError != QCborError::NoError)
    {
        qWarning() << "Failed to reader:" << fileName << "error:" << cborError.toString();
        return nullptr;
    }

    QJsonValue jsonValue = cbor.toJsonValue();

    if (!jsonValue.isObject())
    {
        qWarning() << "JSON value is not an object";
        return nullptr;
    }

    QJsonObject json = jsonValue.toObject();

    try {
        auto replay = PostFeedReplay::fromJson(json);
        qDebug() << "Loaded:" << fileName;
        return replay;
    } catch (ATProto::InvalidJsonException& e) {
        qWarning() << e.msg();
        return nullptr;
    }

    return nullptr;
}

void PostFeedReplay::finishLoad()
{
    sLoadThread->wait();
    sLoadThread = nullptr;

    if (sLoadCb)
        sLoadCb(sLoadedReplay);

    sLoadCb = {};
    sLoadedReplay = nullptr;
}

}
