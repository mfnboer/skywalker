// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "search_utils.h"
#include <QTextBoundaryFinder>

namespace Skywalker {

std::set<QString> SearchUtils::getWords(const QString& text)
{
    if (text.isEmpty())
        return {};

    std::set<QString> words;
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Word, text);
    int startWordPos = -1;

    if (boundaryFinder.boundaryReasons() & QTextBoundaryFinder::StartOfItem)
        startWordPos = boundaryFinder.position();
    else
        startWordPos = boundaryFinder.toNextBoundary();

    while (startWordPos != -1)
    {
        int endWordPos = startWordPos;

        if (!(boundaryFinder.boundaryReasons() & QTextBoundaryFinder::EndOfItem))
            endWordPos = boundaryFinder.toNextBoundary();

        Q_ASSERT(endWordPos != -1);
        if (endWordPos == -1)
            break;

        const QString word = text.sliced(startWordPos, endWordPos - startWordPos + 1);
        words.insert(word);

        startWordPos = boundaryFinder.toNextBoundary();
    }

    return words;
}

SearchUtils::SearchUtils(QObject* parent) :
    Presence(),
    QObject(parent)
{
}

void SearchUtils::setSkywalker(Skywalker* skywalker)
{
    Q_ASSERT(skywalker);
    mSkywalker = skywalker;
    emit skywalkerChanged();
}

void SearchUtils::setAuthorTypeaheadList(const BasicProfileList& list)
{
    mAuthorTypeaheadList = list;
    emit authorTypeaheadListChanged();
}

void SearchUtils::setAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasicList& profileViewBasicList)
{
    BasicProfileList list;

    for (const auto& profile : profileViewBasicList)
    {
        BasicProfile basicProfile(profile.get());
        list.append(basicProfile.nonVolatileCopy());
    }

    setAuthorTypeaheadList(list);
}

void SearchUtils::searchAuthorsTypeahead(const QString& prefix)
{
    bskyClient()->searchActorsTypeahead(prefix, 20,
        [this, presence=getPresence()](auto searchOutput){
            if (!presence)
                return;

            setAuthorTypeaheadList(searchOutput->mActors);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qWarning() << "Type ahaed search failed:" << error;
        });
}

ATProto::Client* SearchUtils::bskyClient()
{
    Q_ASSERT(mSkywalker);
    auto* client = mSkywalker->getBskyClient();
    Q_ASSERT(client);
    return client;
}

}
