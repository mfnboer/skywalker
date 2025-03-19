// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>

namespace Skywalker {

enum class TextDiffType
{
    NONE,
    INSERTED,
    DELETED,
    REPLACED
};

class TextDiffer
{
public:
    struct Result
    {
        TextDiffType mType;
        int mOldStartIndex = -1;
        int mOldEndIndex = -1; // inclusive
        int mNewStartIndex = -1;
        int mNewEndIndex = -1; // inclusive
    };

    static Result diff(const QString& oldText, const QString& newText);
};

}
