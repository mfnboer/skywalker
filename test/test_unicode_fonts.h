// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <unicode_fonts.h>
#include <QtTest/QTest>

using namespace Skywalker;

class TestUnicodeFonts : public QObject
{
    Q_OBJECT
private slots:
    void splitText_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("maxLength");
        QTest::addColumn<QStringList>("output");

        QTest::newRow("exact 1") << "abcd efgh"               << 4 << QStringList{"abcd ", "efgh"};
        QTest::newRow("exact 2") << "abcd efgh ijkl"          << 4 << QStringList{"abcd ", "efgh ", "ijkl"};
        QTest::newRow("overmatch") << "abcd, efgh"            << 4 << QStringList{"abcd, ", "efgh"};
        QTest::newRow("oversize") << "abcd efgh ijklm"        << 5 << QStringList{"abcd ", "efgh ", "ijklm"};
        QTest::newRow("trailing whitespace 1") << "abc "      << 4 << QStringList{"abc "};
        QTest::newRow("trailing whitespace 2") << "abcd efg " << 4 << QStringList{"abcd ", "efg "};
        QTest::newRow("trailing whitespace 3") << "abcd "     << 4 << QStringList{"abcd "};
        QTest::newRow("trailing whitespace 4") << "abcd  "    << 4 << QStringList{"abcd  "};
        QTest::newRow("trailing whitespace 5") << "abcd  "    << 5 << QStringList{"abcd  "};
        QTest::newRow("starting whitespace") << "  abcd"      << 4 << QStringList{"  ", "abcd"};
        QTest::newRow("whitespace") << "abcd  e"              << 5 << QStringList{"abcd  ", "e"};
        QTest::newRow("newline") << "abcd\nefgh"              << 4 << QStringList{"abcd\n", "efgh"};
        QTest::newRow("tab") << "abcd\tefgh"                  << 4 << QStringList{"abcd\t", "efgh"};
    }

    void splitText()
    {
        QFETCH(QString, text);
        QFETCH(int, maxLength);
        QFETCH(QStringList, output);
        QCOMPARE(UnicodeFonts::splitText(text, maxLength, 0), output);
    }

    void splitTextMinSplitLineLength_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("minSplitLineLength");
        QTest::addColumn<QStringList>("output");

        QTest::newRow("split 1") << "aa\nbb\nc\nd d\n" << 3 << QStringList{"aa\nbb\nc\n", "d d\n"};
        QTest::newRow("split 2") << "aa\nbb\nc\nd d\n" << 2 << QStringList{"aa\nbb\nc\nd ", "d\n"};
    }

    void splitTextMinSplitLineLength()
    {
        QFETCH(QString, text);
        QFETCH(int, minSplitLineLength);
        QFETCH(QStringList, output);
        QCOMPARE(UnicodeFonts::splitText(text, 10, minSplitLineLength), output);
    }

    void splitTextMaxParts_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("maxParts");
        QTest::addColumn<QStringList>("output");

        QTest::newRow("split 1") << "aa bb cc dd" << 2 << QStringList{"aa ", "bb cc dd"};
        QTest::newRow("split 2") << "aa bb cc dd" << 3 << QStringList{"aa ", "bb ", "cc dd"};
    }

    void splitTextMaxParts()
    {
        QFETCH(QString, text);
        QFETCH(int, maxParts);
        QFETCH(QStringList, output);
        QCOMPARE(UnicodeFonts::splitText(text, 3, 0, maxParts), output);
    }
};

