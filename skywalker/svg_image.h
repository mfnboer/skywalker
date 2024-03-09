// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class SvgImage
{
    Q_GADGET
    Q_PROPERTY(QString path MEMBER mPath CONSTANT FINAL)
    Q_PROPERTY(int width MEMBER mWidth CONSTANT FINAL)
    Q_PROPERTY(int height MEMBER mHeight CONSTANT FINAL)
    QML_VALUE_TYPE(svgimage)

public:
    SvgImage() = default;
    constexpr SvgImage(const char* path, int width = 960, int height = 960) :
        mPath(path),
        mWidth(width),
        mHeight(height)
    {}

    Q_INVOKABLE bool isNull() const { return mWidth == 0; }

private:
    const char* mPath = "";
    int mWidth = 0;
    int mHeight = 0;
};

}

Q_DECLARE_METATYPE(::Skywalker::SvgImage)
