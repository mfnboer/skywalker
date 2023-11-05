// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "invite_code.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class InviteCodeStore : public WrappedSkywalker
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit InviteCodeStore(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE int getAvailableCount() const { return mAvailableCount; }
    Q_INVOKABLE const QVariantList getCodes() const;

    void retrieveUsedByProfile(InviteCode& code);
    void codeCopied();

signals:
    void loaded();

private:
    InviteCodeList mCodes;
    int mAvailableCount = 0;
};

}
