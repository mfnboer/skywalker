// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/client.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class Skywalker : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit Skywalker(QObject* parent = nullptr);

    Q_INVOKABLE void login(const QString user, QString password, const QString host);

signals:
    void loginOk();
    void loginFailed(QString error);

private:
    std::unique_ptr<ATProto::Client> mBsky;
};

}
