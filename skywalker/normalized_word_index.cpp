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

        for (const auto& tag : hashtagList)
        {
            const auto normalizedTag = SearchUtils::normalizeText(tag);
            const_cast<NormalizedWordIndex*>(this)->mHashtags.insert(normalizedTag);
        }
    }

    return mHashtags;
}

const std::vector<QString>& NormalizedWordIndex::getNormalizedWords() const
{
    if (mNormalizedWords.empty())
    {
        auto& normalizeWords = const_cast<NormalizedWordIndex*>(this)->mNormalizedWords;
        normalizeWords = SearchUtils::getNormalizedWords(getText());

        const auto& imageViews = getImages();

        for (const auto& imageView : imageViews)
        {
            const auto normalizedAlt = SearchUtils::getNormalizedWords(imageView.getAlt());
            normalizeWords.insert(normalizeWords.end(), normalizedAlt.begin(), normalizedAlt.end());
        }

        const auto& videoView = getVideoView();

        if (videoView)
        {
            const auto normalizedAlt = SearchUtils::getNormalizedWords(videoView->getAlt());
            normalizeWords.insert(normalizeWords.end(), normalizedAlt.begin(), normalizedAlt.end());
        }

        const auto& externalView = getExternalView();

        if (externalView)
        {
            const auto normalizedTitle = SearchUtils::getNormalizedWords(externalView->getTitle());
            normalizeWords.insert(normalizeWords.end(), normalizedTitle.begin(), normalizedTitle.end());
            const auto normalizedDescription = SearchUtils::getNormalizedWords(externalView->getDescription());
            normalizeWords.insert(normalizeWords.end(), normalizedDescription.begin(), normalizedDescription.end());
        }
    }

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
