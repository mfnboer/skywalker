// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "skywalker.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class PostUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Skywalker* skywalker READ getSkywalker WRITE setSkywalker NOTIFY skywalkerChanged FINAL REQUIRED)
    QML_ELEMENT

public:
    explicit PostUtils(QObject* parent = nullptr);

    Q_INVOKABLE QString highlightMentionsAndLinks(const QString& text);
    Q_INVOKABLE int graphemeLength(const QString& text);

    Skywalker* getSkywalker() const { return mSkywalker; }
    void setSkywalker(Skywalker* skywalker);

signals:
    void skywalkerChanged();

private:
    Skywalker* mSkywalker = nullptr;
};

}
