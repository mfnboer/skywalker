// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class AccountUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool emailUpdateInProgress READ getEmailUpdateInProgress NOTIFY emailUpdateInProgressChanged FINAL)
    QML_ELEMENT

public:
    explicit AccountUtils(QObject* parent = nullptr);

    Q_INVOKABLE void update2FA(bool enable, const QString& token);
    Q_INVOKABLE void requestEmailUpdateToken();

signals:
    void update2FAOk(bool enabled);
    void update2FAFailed(QString error);
    void emailUpdateTokenOk();
    void emailUpdateTokenNotRequired();
    void emailUpdateTokenFailed(QString error);
    void emailUpdateInProgressChanged();

private:
    bool getEmailUpdateInProgress() const { return mEmailUpdateInProgress; }
    void setEmailUpdateInProgress(bool inProgress);

    bool mEmailUpdateInProgress = false;
};

}
