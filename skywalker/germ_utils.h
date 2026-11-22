// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class GermUtils : public WrappedSkywalker
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit GermUtils(QObject* parent = nullptr);

    Q_INVOKABLE bool mustShowButton(const BasicProfile& profile) const;
    Q_INVOKABLE QString getMessageUrl(const BasicProfile& profile) const;

private:
    QString createDmUrl(const BasicProfile& profile, const QString& messageMeUrl) const;
};

}