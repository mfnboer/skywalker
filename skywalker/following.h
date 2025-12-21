// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>

namespace Skywalker {

class Following : public QObject
{
    Q_OBJECT

public:
    explicit Following(QObject* parent = nullptr);

    void follow(const QString& did);
    void unfollow(const QString& did);

signals:
    void startedFollowing(const QString& did);
    void stoppedFollowing(const QString& did);
};

}
