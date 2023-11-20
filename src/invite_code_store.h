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

    Q_INVOKABLE void clear();
    Q_INVOKABLE void load(QDateTime prevSignIn);
    Q_INVOKABLE int getAvailableCount() const { return mAvailableCount; }
    Q_INVOKABLE const QVariantList getCodes() const;
    Q_INVOKABLE bool failedToLoad() const { return mFailedToLoad; }

    const InviteCodeList& getUsedSincePreviousSignIn() const { return mUsedSincePreviousSignIn; }
    void retrieveUsedByProfile(InviteCode& code);
    void codeCopied();

signals:
    void loaded();

private:
    InviteCodeList mCodes;
    InviteCodeList mUsedSincePreviousSignIn;
    int mAvailableCount = 0;
    bool mFailedToLoad = false;
};

}
