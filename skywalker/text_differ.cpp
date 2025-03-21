// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "text_differ.h"

namespace Skywalker {

TextDiffer::Result TextDiffer::diff(const QString& oldText, const QString& newText)
{
    Result result;

    for (int i = 0 ; i < newText.size(); ++i)
    {
        if (i > oldText.size() - 1 || newText[i] != oldText[i])
        {
            result.mNewStartIndex = i;
            break;
        }
    }

    if (result.mNewStartIndex < 0)
    {
        if (oldText.size() == newText.size())
        {
            result.mType = TextDiffType::NONE;
        }
        else
        {
            result.mType = TextDiffType::DELETED;
            result.mOldStartIndex = newText.size();
            result.mOldEndIndex = oldText.size() - 1;
        }

        return result;
    }

    for (int i = newText.size() - 1; i >= result.mNewStartIndex; --i)
    {
        const int oldIndex = oldText.size() - (newText.size() - i);

        if (oldIndex < result.mNewStartIndex || newText[i] != oldText[oldIndex])
        {
            result.mNewEndIndex = i;
            break;
        }
    }

    if (result.mNewEndIndex < 0)
    {
        result.mType = TextDiffType::DELETED;
        result.mOldStartIndex = result.mNewStartIndex;
        result.mOldEndIndex = oldText.size() - 1 - (newText.size() - result.mNewStartIndex);
        return result;
    }

    if (result.mNewStartIndex + newText.size() - 1 - result.mNewEndIndex == oldText.size())
    {
        result.mType = TextDiffType::INSERTED;
        return result;
    }

    result.mType = TextDiffType::REPLACED;
    result.mOldStartIndex = result.mNewStartIndex;
    result.mOldEndIndex = oldText.size() - (newText.size() - result.mNewEndIndex);
    return result;
}

}
