// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "words_highlighter.h"
#include "search_utils.h"

namespace Skywalker {

static QString htmlReplace(const QString& html, const QString& oldWord, const QString& newWord)
{
    qDebug() << "Replace:" << oldWord << "by:" << newWord;
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

    QString pattern = oldWord.startsWith('#') ?
        QString("%1\\b").arg(QRegularExpression::escape(oldWord)) :
        QString("\\b%1\\b").arg(QRegularExpression::escape(oldWord));

    QRegularExpression wordRe(pattern);

    if (matches.empty())
    {
        // Text has not html tags
        result.replace(wordRe, newWord);
        return result;
    }

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

    const auto combinedLetterWords = SearchUtils::combineSingleCharsToWords(normalizedWords);

    for (const auto& w : combinedLetterWords)
    {
        qDebug() << "Combined:" << w;
        QString pattern;

        for (auto ch : w)
        {
            if (pattern.isEmpty())
                pattern += "\\b";
            else
                pattern += "\\W+";

            pattern += ch;
        }

        pattern += "\\b";

        QRegularExpression combinedRe(pattern);
        QRegularExpressionMatchIterator it = combinedRe.globalMatch(text);

        if (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            QString combinedWord = text.mid(match.capturedStart(), match.capturedLength());
            normalizedWordsMap[w].insert(combinedWord);
            qDebug() << "Combined entry:" << w << "->" << combinedWord;
        }
    }

    return normalizedWordsMap;
}

static std::unordered_map<QString, std::unordered_set<QString>> getNormalizedHashtagsMap(const QString& text)
{
    std::unordered_map<QString, std::unordered_set<QString>> normalizedHashtagsMap;
    const auto hashtags = UnicodeFonts::extractHashtags(text);
    const auto normalizedHashtags = normalizeWords(hashtags);
    Q_ASSERT(hashtags.size() == normalizedHashtags.size());

    for (int i = 0; i < (int)hashtags.size(); ++i)
        normalizedHashtagsMap[normalizedHashtags[i]].insert(hashtags[i]);

    return normalizedHashtagsMap;
}

WordsHighlighter::WordsHighlighter(QObject* parent) :
    QObject(parent)
{
}

QString WordsHighlighter::highlight(const QString& text, const QString& words, const QString& color, bool isHashtag) const
{
    qDebug() << "text:" << text << "words:" << words;

    QString hightlightedText = text;
    const auto normalizedWords = isHashtag ? std::vector<QString>{words} : normalizeWords(SearchUtils::getWords(words));
    const auto textNormalizedWordsMap = isHashtag ? getNormalizedHashtagsMap(text) : getNormalizedWordsMap(text);

    for (const auto& word : normalizedWords)
    {
        const auto it = textNormalizedWordsMap.find(word);

        if (it == textNormalizedWordsMap.end())
        {
            qDebug() << "Word not found:" << word;
            return text;
        }

        const std::unordered_set<QString>& textWords = it->second;

        for (const QString& textWord : textWords)
        {
            const QString highlightedWord = QString("<span style=\"background-color:%1\">%2</span>").arg(color, textWord);
            hightlightedText = htmlReplace(hightlightedText, textWord, highlightedWord);
        }
    }

    qDebug() << "highlighted:" << hightlightedText;
    return hightlightedText;
}

}
