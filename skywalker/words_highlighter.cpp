// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "words_highlighter.h"
#include "search_utils.h"

namespace Skywalker {

static QString plainReplace(const QString& plain, const QString& oldWord, const QString& newWord)
{
    QString result = plain;
    QString pattern = QString("\\b%1\\b").arg(QRegularExpression::escape(oldWord));
    QRegularExpression wordRe(pattern);
    result.replace(wordRe, newWord);

    return result;
}

static QString htmlReplace(const QString& html, const QString& oldWord, const QString& newWord)
{
    QString result = html;

    // Pattern explanation:
    // (?<=>)[^<>]*(?=<)  - text between tags: > text
    // (?<=>)[^<>]*$      - text after last tag: > text (end)
    // ^[^<>]*(?=<)       - text before first tag: (start) text

    static const QRegularExpression re("(?<=>)[^<>]*(?=<)|(?<=>)[^<>]*$|^[^<>]*(?=<)");
    QRegularExpressionMatchIterator it = re.globalMatch(html);

    // Collect matches in reverse order to maintain positions
    std::vector<std::pair<int, int>> matches;

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        matches.push_back({ match.capturedStart(), match.capturedLength() });
    }

    QString pattern = QString("\\b%1\\b").arg(QRegularExpression::escape(oldWord));
    QRegularExpression wordRe(pattern);

    // Replace from end to beginning to maintain offsets
    for (int i = matches.size() - 1; i >= 0; --i)
    {
        int pos = matches[i].first;
        int len = matches[i].second;
        QString textPart = result.mid(pos, len);
        textPart.replace(wordRe, newWord);

        result.replace(pos, len, textPart);
    }

    return result;
}

static std::vector<QString> normalizeWords(const std::vector<QString>& words)
{
    std::vector<QString> normalizedWords;
    normalizedWords.reserve(words.size());

    for (const auto& word : words)
        normalizedWords.push_back(SearchUtils::normalizeText(word));

    return normalizedWords;
}

static std::unordered_map<QString, std::unordered_set<QString>> getNormalizedWordsMap(const QString& text)
{
    std::unordered_map<QString, std::unordered_set<QString>> normalizedWordsMap; // normalized word -> word

    const auto words = SearchUtils::getWords(text);
    const auto normalizedWords = normalizeWords(words);
    Q_ASSERT(words.size() == normalizedWords.size());

    for (int i = 0; i < (int)words.size(); ++i)
        normalizedWordsMap[normalizedWords[i]].insert(words[i]);

    // TODO combined words from single letters
    return normalizedWordsMap;
}

WordsHighlighter::WordsHighlighter(QObject* parent) :
    QObject(parent)
{
}

QString WordsHighlighter::highlight(const QString& text, const QString& words, const QString& color, bool html) const
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

        const std::unordered_set<QString>& textWords = it->second;

        for (const QString& textWord : textWords)
        {
            const QString highlightedWord = QString("<span style=\"background-color:%1\">%2</span>").arg(color, textWord);
            hightlightedText = html ?
                    htmlReplace(hightlightedText, textWord, highlightedWord) :
                    plainReplace(hightlightedText, textWord, highlightedWord);
        }
    }

    return hightlightedText;
}

}
