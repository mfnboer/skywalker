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
    QML_ELEMENT
    QML_UNCREATABLE("only created on the C++ side")

public:
    SvgImage(QObject* parent = nullptr) : QObject(parent) {}
    SvgImage(const char* path, QObject* parent = nullptr, int width = 960, int height = 960) :
        QObject(parent),
        mPath(path),
        mWidth(width),
        mHeight(height)
    {}

    Q_INVOKABLE bool isNull() const { return mWidth == 0; }

private:
    QString mPath = "";
    int mWidth = 0;
    int mHeight = 0;
};

}

Q_DECLARE_METATYPE(::Skywalker::SvgImage)
