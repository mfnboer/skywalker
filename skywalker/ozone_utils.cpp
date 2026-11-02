// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_utils.h"
#include "skywalker.h"
#include "ozone_report_list_model.h"
#include "utils.h"

namespace Skywalker {

OzoneUtils::OzoneUtils(QObject* parent) :
    WrappedSkywalker(parent)
{}

void OzoneUtils::getReportList(int id, int limit, int maxPages, int minEntries,  const QString& cursor)
{
    qDebug() << "Get report list:" << id << "cursor:" << cursor;

    if (!bskyClient())
        return;

    auto* model = mSkywalker->getOzoneReportListModel(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get author starter pack list still in progress";
        return;
    }

    model->setGetFeedInProgress(true);

    bskyClient()->ozoneInboxListReports(model->getReportsFilter(), {}, {}, {}, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), id, limit, maxPages, minEntries, cursor](ATProto::ToolsOzoneInbox::ListReportsOutput::SharedPtr output){
            if (!presence)
                return;

            auto* model = mSkywalker->getOzoneReportListModel(id);


            if (!model)
                return; // user has closed the view

            model->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                model->clear();

            const int added = model->addReports(std::move(output->mReports), output->mCursor.value_or(""));
            const int toAdd = minEntries - added;

            if (toAdd < 0)
                getReportListNextPage(id, limit, maxPages - 1, toAdd);
        },
        [this, presence=getPresence(), id](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getActorStarterPacks failed:" << error << " - " << msg;
            auto* model = mSkywalker->getOzoneReportListModel(id);

            if (model)
                model->setGetFeedInProgress(false);

            mSkywalker->statusMessage(mSkywalker->getUserDid(), msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void OzoneUtils::getReportListNextPage(int id, int limit, int maxPages, int minEntries)
{
    qDebug() << "Get report list next page:" << id;

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    auto* model = mSkywalker->getOzoneReportListModel(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    const auto& cursor = model->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    getReportList(id, limit, maxPages, minEntries, cursor);
}

}