// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "words_highlighter.h"
#include "search_utils.h"

namespace Skywalker {

static std::vector<QString> normalizeWords(const std::vector<QString>& words)
{
    std::vector<QString> normalizedWords;
    normalizedWords.reserve(words.size());

    for (const auto& word : words)
        normalizedWords.push_back(SearchUtils::normalizeText(word));

    return normalizedWords;
}

static std::unordered_map<QString, QString> getNormalizedWordsMap(const QString& text)
{
    std::unordered_map<QString, QString> normalizedWordsMap; // normalized word -> word

    const auto words = SearchUtils::getWords(text);
    const auto normalizedWords = normalizeWords(words);
    Q_ASSERT(words.size() == normalizedWords.size());

    for (int i = 0; i < (int)words.size(); ++i)
        normalizedWordsMap[normalizedWords[i]] = words[i];

    // TODO combined words from single letters
    return normalizedWordsMap;
}

WordsHighlighter::WordsHighlighter(QObject* parent) :
    QObject(parent)
{
}

QString WordsHighlighter::highlight(const QString& text, const QString& words, const QString& color) const
{
    qDebug() << "text:" << text << "words:" << words;

    QString hightlightedText = text;
    const auto normalizedWords = normalizeWords(SearchUtils::getWords(words));
    const auto textNormalizedWordsMap = getNormalizedWordsMap(text);

    for (const auto& word : normalizedWords)
    {
        const auto it = textNormalizedWordsMap.find(word);

        if (it == textNormalizedWordsMap.end())
        {
            qDebug() << "Word not found:" << word;
            continue;
        }

        const QString textWord = it->second;
        const QString highlightedWord = QString("<span style=\"background-color:%1\">%2</span>").arg(color, textWord);
        hightlightedText.replace(textWord, highlightedWord);
    }

    return hightlightedText;
}

}
