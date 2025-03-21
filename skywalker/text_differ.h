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
    // Indexes in old text for deleted part
    // Indexes in new text for inserted part
    // Replacement is a combination of delete and insert
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
