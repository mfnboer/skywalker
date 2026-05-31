// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class SvgImage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString path MEMBER mPath CONSTANT FINAL)
    Q_PROPERTY(int width MEMBER mWidth CONSTANT FINAL)
    Q_PROPERTY(int height MEMBER mHeight CONSTANT FINAL)
    Q_PROPERTY(bool offsetByHeight MEMBER mOffsetByHeight CONSTANT FINAL)
    QML_ELEMENT
    QML_UNCREATABLE("only created on the C++ side")

public:
    SvgImage(QObject* parent = nullptr) : QObject(parent) {}

    // The SVGs from Google have view port of 960x960 and the origin is in the lower left.
    // Qt has its origin in the upper left, so the y-coordinate must be offset by the height.
    SvgImage(const char* path, QObject* parent = nullptr, int width = 960, int height = 960, bool offsetByHeight = true) :
        QObject(parent),
        mPath(path),
        mWidth(width),
        mHeight(height),
        mOffsetByHeight(offsetByHeight)
    {}

    Q_INVOKABLE bool isNull() const { return mWidth == 0; }

private:
    QString mPath = "";
    int mWidth = 0;
    int mHeight = 0;
    bool mOffsetByHeight = false;
};

}

Q_DECLARE_METATYPE(::Skywalker::SvgImage)
