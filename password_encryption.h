// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QDebug>
#include <QString>
#include <unordered_map>

namespace Skywalker
{

class PasswordEncryption
{
public:
    bool init(const QString& keyAlias);
    QByteArray encrypt(const QString& token, const QString& keyAlias);
    QString decrypt(const QByteArray& token, const QString& keyAlias);

private:
    std::unordered_map<QString, bool> mKeyAliasIntialized;
};

}
