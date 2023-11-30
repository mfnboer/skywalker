// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "normalized_word_index.h"
#include "search_utils.h"

namespace Skywalker {

const std::unordered_set<QString>& NormalizedWordIndex::getUniqueHashtags() const
{
    if (mHashtags.empty())
    {
        const auto& hashtagList = getHashtags();
        const_cast<NormalizedWordIndex*>(this)->mHashtags.insert(hashtagList.begin(), hashtagList.end());
    }

    return mHashtags;
}

const std::vector<QString>& NormalizedWordIndex::getNormalizedWords() const
{
    if (mNormalizedWords.empty())
        const_cast<NormalizedWordIndex*>(this)->mNormalizedWords = SearchUtils::getWords(getText());

    return mNormalizedWords;
}

const std::unordered_map<QString, std::vector<int>>& NormalizedWordIndex::getUniqueNormalizedWords() const
{
    if (mUniqueNormalizedWords.empty())
    {
        const auto& normalizedWords = getNormalizedWords();

        for (int i = 0; i < (int)normalizedWords.size(); ++i)
        {
            const QString& word = normalizedWords[i];
            const_cast<NormalizedWordIndex*>(this)->mUniqueNormalizedWords[word].push_back(i);
        }
    }

    return mUniqueNormalizedWords;
}

}
