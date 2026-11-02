// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QAbstractListModel>

namespace Skywalker {

// A templated class cannot have Q_OBJECT itself, therefore we have this abstract base
class AbstractBaseListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)

public:
    explicit AbstractBaseListModel(const QString& userDid, QObject* parent = nullptr);
    virtual ~AbstractBaseListModel() = default;

    virtual void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

signals:
    void getFeedInProgressChanged();

protected:
    const QString& mUserDid;
    bool mGetFeedInProgress = false;
};

}