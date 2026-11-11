// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "ozone_account_status.h"
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class OzoneInbox : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(OzoneAccountStatus userAccountStatus READ getUserAccountStatus NOTIFY userAccountStatusChanged FINAL)
    QML_ELEMENT

public:
    explicit OzoneInbox(QObject* parent = nullptr);

    void reset();
    OzoneAccountStatus getUserAccountStatus() const { return mUserAccountStatus; }
    void updateUserAccountStatus();

signals:
    void userAccountStatusChanged();

private:
    void setUserAccountStatus(const OzoneAccountStatus& status);

    OzoneAccountStatus mUserAccountStatus;
};

}