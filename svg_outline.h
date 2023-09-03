// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class SvgOutline : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString favorite MEMBER sFavorite CONSTANT FINAL)
    QML_ELEMENT

public:
    explicit SvgOutline(QObject* parent = nullptr) : QObject(parent) {}

private:
    static constexpr const char* sFavorite = "M760-200v-160q0-50-35-85t-85-35H273l144 144-57 56-240-240 240-240 57 56-144 144h367q83 0 141.5 58.5T840-360v160h-80Z";
};

}
