// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "svg_image.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class SvgFilled : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SvgImage like MEMBER sFavorite CONSTANT FINAL)
    Q_PROPERTY(SvgImage unknownAvatar MEMBER sPerson CONSTANT FINAL)
    QML_ELEMENT

public:
    explicit SvgFilled(QObject* parent = nullptr) : QObject(parent) {}

private:
    // fonts.googlecom weight=100, grade=0, optical size=24px
    static constexpr SvgImage sFavorite{"m480-190-22-20q-97-89-160.5-152t-100-110.5Q161-520 146.5-558T132-634q0-71 48.5-119.5T300-802q53 0 99 28.5t81 83.5q35-55 81-83.5t99-28.5q71 0 119.5 48.5T828-634q0 38-14.5 76t-51 85.5Q726-425 663-362T502-210l-22 20Z"};
    static constexpr SvgImage sPerson{"M480-512q-44.55 0-76.275-31.725Q372-575.45 372-620q0-44.55 31.725-76.275Q435.45-728 480-728q44.55 0 76.275 31.725Q588-664.55 588-620q0 44.55-31.725 76.275Q524.55-512 480-512ZM212-232v-52q0-22 13.5-41.5T262-356q55-26 109.5-39T480-408q54 0 108.5 13T698-356q23 11 36.5 30.5T748-284v52H212Z"};
};

}
