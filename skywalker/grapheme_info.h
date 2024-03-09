// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class GraphemeInfo
{
    Q_GADGET
    Q_PROPERTY(int length READ getLength FINAL)
    QML_VALUE_TYPE(graphemeinfo)

public:
    GraphemeInfo() = default;
    GraphemeInfo(int length, const std::vector<int>& charPositions);

    int getLength() const { return mLength; }
    Q_INVOKABLE int getCharPos(int graphemeIndex) const;
    Q_INVOKABLE QString sliced(const QString& text, int startIndex, int sz);
    Q_INVOKABLE QString sliced(const QString& text, int startIndex);

private:
    int mLength = 0;
    std::vector<int> mCharPositions;
};

}

Q_DECLARE_METATYPE(::Skywalker::GraphemeInfo)
