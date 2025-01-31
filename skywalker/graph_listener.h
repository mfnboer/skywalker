// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <QObject>

namespace Skywalker {

class GraphListener : public QObject
{
    Q_OBJECT

public:
    static GraphListener& instance();
    explicit GraphListener(QObject* parent = nullptr);

signals:
    void listDeleted(const QString& uri);
    void userAdded(const QString& uri, const BasicProfile& profile, const QString& itemUri);
    void userRemoved(const QString& uri, const QString& itemUri);

private:
    static std::unique_ptr<GraphListener> sInstance;
};

}
