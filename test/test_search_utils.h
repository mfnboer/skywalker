// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <search_utils.h>
#include <QTest>

using namespace Skywalker;

class TestSearchUtils : public QObject
{
    Q_OBJECT
private slots:
    void normalizeText_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("output");
        QTest::newRow("collation") << "áèïõ" << "aeio";
        QTest::newRow("casing") << "ABCdef" << "abcdef";
        QTest::newRow("non-letters") << "Hello, world!" << "hello, world!";
        QTest::newRow("emoji") << "😅😂🤣" << "😅😂🤣";
        QTest::newRow("bold sans") << "𝗔𝗕𝗖𝗗𝗘𝗙𝗚𝗛𝗜𝗝𝗞𝗟𝗠𝗡𝗢𝗣𝗤𝗥𝗦𝗧𝗨𝗩𝗪𝗫𝗬𝗭𝟬𝟭𝟮𝟯𝟰𝟱𝟲𝟳𝟴𝟵" << "abcdefghijklmnopqrstuvwxyz0123456789";
        QTest::newRow("math A") << "𝒜" << "a";
    }

    void normalizeText()
    {
        QFETCH(QString, input);
        QFETCH(QString, output);
        QCOMPARE(SearchUtils::normalizeText(input), output);
    }

    void getWords_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<std::vector<QString>>("output");
        QTest::newRow("empty") << "" << std::vector<QString>{};
        QTest::newRow("single word") << "hello" << std::vector<QString>{"hello"};
        QTest::newRow("multiple words") << "Hello beautiful WORLD" << std::vector<QString>{"hello", "beautiful", "world"};
        QTest::newRow("punctuation") << "Hello, beautiful WORLD!!" << std::vector<QString>{"hello", "beautiful", "world"};
        QTest::newRow("emoji") << "😀Hello, beautiful 😀 WORLD😅😂🤣" << std::vector<QString>{"hello", "beautiful", "world"};
        QTest::newRow("no words") << "😀, 😀 !😅😂🤣.." << std::vector<QString>{};
        QTest::newRow("combine single letters 1") << "s k y walker" << std::vector<QString>{"s", "k", "y", "walker", "sky"};
        QTest::newRow("combine single letters 2") << "S K Y" << std::vector<QString>{"s", "k", "y", "sky"};
        QTest::newRow("no combine") << "H i" << std::vector<QString>{"h", "i"};
    }

    void getWords()
    {
        QFETCH(QString, input);
        QFETCH(std::vector<QString>, output);
        QCOMPARE(SearchUtils::getWords(input), output);
    }
};

QTEST_MAIN(TestSearchUtils)
