// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "backlinks.h"
#include "skywalker.h"
#include "utils.h"

namespace Skywalker {

Backlinks::Backlinks(Constellation& constellation, QObject* parent) :
    WrappedSkywalker(parent),
    mConstellation(constellation)
{
}

void Backlinks::getBlockedByAuthorList(const QString& atId, const QString& cursor, int modelId)
{
    Q_ASSERT(mSkywalker);
    if (!mSkywalker)
        return;

    auto* model = mSkywalker->getAuthorListModel(modelId);

    if (model)
        model->setGetFeedInProgress(true);

    mConstellation.getBackLinks(atId, "app.bsky.graph.block:subject", {}, 25, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), modelId](Constellation::Backlinks::SharedPtr backlinks){
            if (!presence)
                return;

            std::vector<QString> dids;

            for (const auto& record : backlinks->mRecords)
                dids.push_back(record->mDid);

            getProfiles(dids, backlinks->mCursor.value_or(""), modelId);
        },
        [this, presence=getPresence(), modelId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "getBlockedByAuthorList failed:" << error << "-" << msg;
            auto* model = mSkywalker->getAuthorListModel(modelId);

            if (model)
                model->setGetFeedInProgress(false);

            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Backlinks::getProfiles(const std::vector<QString>& dids, const QString& cursor, int modelId)
{
    Q_ASSERT(mSkywalker);
    if (!mSkywalker)
        return;

    if (dids.empty())
    {
        qDebug() << "Not profiles to get, modelId:" << modelId;
        auto* model = mSkywalker->getAuthorListModel(modelId);

        if (model)
        {
            model->setGetFeedInProgress(false);
            model->setCursor(cursor);
        }

        return;
    }

    bskyClient()->getProfiles(dids,
        [this, presence=getPresence(), cursor, modelId](auto profileDetailedList){
            if (!presence)
                return;

            auto* model = mSkywalker->getAuthorListModel(modelId);

            if (model)
            {
                model->setGetFeedInProgress(false);
                model->addAuthors(std::move(profileDetailedList), cursor);
            }
        },
        [this, presence=getPresence(), modelId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "getProfiles failed:" << error << "-" << msg;
            auto* model = mSkywalker->getAuthorListModel(modelId);

            if (model)
                model->setGetFeedInProgress(false);

            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

}