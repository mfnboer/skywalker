// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>
#include <QQmlEngine>

namespace Skywalker {

class OzonePolicySummary {
    Q_GADGET
    Q_PROPERTY(QString decription READ getDescription FINAL)
    Q_PROPERTY(QString link READ getLink FINAL)
    QML_VALUE_TYPE(ozonepolicysummary)

public:
    using List = QList<OzonePolicySummary>;

    OzonePolicySummary() = default;
    explicit OzonePolicySummary(const QString& description, const QString& link = {});

    const QString& getDescription() const { return mDescription; }
    const QString& getLink() const { return mLink; }

private:
    QString mDescription;
    QString mLink;
};

}
