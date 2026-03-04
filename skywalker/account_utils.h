// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class AccountUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool updateInProgress READ getUpdateInProgress NOTIFY updateInProgressChanged FINAL)
    QML_ELEMENT

public:
    explicit AccountUtils(QObject* parent = nullptr);

    Q_INVOKABLE void update2FA(bool enable, const QString& token);
    Q_INVOKABLE void requestEmailUpdateToken();
    Q_INVOKABLE void resetPassword(const QString& password, const QString& token);
    Q_INVOKABLE void requestPasswordReset(QString email = {});

signals:
    void update2FAOk(bool enabled);
    void update2FAFailed(QString error);
    void emailUpdateTokenOk();
    void emailUpdateTokenNotRequired();
    void emailUpdateTokenFailed(QString error);
    void resetPasswordOk();
    void resetPasswordFailed(QString error);
    void requestResetPasswordOk();
    void requestResetPasswordFailed(QString error);
    void updateInProgressChanged();

private:
    bool getUpdateInProgress() const { return mUpdateInProgress; }
    void setUpdateInProgress(bool inProgress);

    bool mUpdateInProgress = false;
};

}
