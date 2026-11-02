// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "abstract_base_list_model.h"

namespace Skywalker {

AbstractBaseListModel::AbstractBaseListModel(const QString& userDid, QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid)
{
}

void AbstractBaseListModel::setGetFeedInProgress(bool inProgress)
{
    if (mGetFeedInProgress != inProgress)
    {
        mGetFeedInProgress = inProgress;
        emit getFeedInProgressChanged();
    }
}

}