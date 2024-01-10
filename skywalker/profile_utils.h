// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "profile.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class ProfileUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ProfileUtils(QObject* parent = nullptr);

    Q_INVOKABLE void getProfileView(const QString& atId);

signals:
    void profileViewOk(Profile profile);
    void profileViewFailed(QString error);
};

}
