// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <text_splitter.h>
#include <QtTest/QTest>

using namespace Skywalker;

class TestTextSplitter : public QObject
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
        QCOMPARE(toStringList(mTextSplitter.splitText(text, {}, maxLength, 0)), output);
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
        QCOMPARE(toStringList(mTextSplitter.splitText(text, {}, 10, minSplitLineLength)), output);
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
        QCOMPARE(toStringList(mTextSplitter.splitText(text, {}, 3, 0, maxParts)), output);
    }

    void splitTextWithLinks_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("maxLength");
        QTest::addColumn<WebLink::List>("links");
        QTest::addColumn<TextSplitterPart::List>("output");

        QTest::newRow("split 1") << "aaaa bbbb cccc dddd" << 10
            << WebLink::List{ WebLink{"bbbb cccc", 5, 14}, WebLink{"dddd", 15, 19} }
            << TextSplitterPart::List{
                    TextSplitterPart{ "aaaa ", {} },
                    TextSplitterPart{ "bbbb cccc ", { WebLink{"bbbb cccc", 0, 9} } },
                    TextSplitterPart{ "dddd", { WebLink{"dddd", 0, 4} } }};

        QTest::newRow("split 2") << "aaaa bbbb cccc dddd" << 10
            << WebLink::List{ WebLink{"dddd", 15, 19} }
            << TextSplitterPart::List{
                    TextSplitterPart{ "aaaa bbbb ", {} },
                    TextSplitterPart{ "cccc dddd", { WebLink{"dddd", 5, 9} } }};
    }

    void splitTextWithLinks()
    {
        QFETCH(QString, text);
        QFETCH(int, maxLength);
        QFETCH(WebLink::List, links);
        QFETCH(TextSplitterPart::List, output);

        const auto parts = mTextSplitter.splitText(text, links, maxLength, 0);
        QCOMPARE(parts.size(), output.size());

        for (int i = 0; i < std::min(parts.size(), output.size()); ++i)
        {
            const auto& part = parts[i];
            const auto& outputPart = output[i];
            qDebug() << "Part:" << i << part.getText();
            QCOMPARE(part.getText(), outputPart.getText());
            QCOMPARE(part.getEmbeddedLinks(), outputPart.getEmbeddedLinks());
        }
    }

    void joinText_data()
    {
        QTest::addColumn<QString>("text1");
        QTest::addColumn<WebLink::List>("links1");
        QTest::addColumn<QString>("text2");
        QTest::addColumn<WebLink::List>("links2");
        QTest::addColumn<TextSplitterPart>("output");

        QTest::newRow("empty") << "" << WebLink::List{} << "" << WebLink::List{} << TextSplitterPart{"", {}};
        QTest::newRow("empty1") << "" << WebLink::List{}
                                << "foo" << WebLink::List{ WebLink{"foo", 0, 3} }
                                << TextSplitterPart{"foo", { WebLink{"foo", 0, 3} }};
        QTest::newRow("empty2") << "foo" << WebLink::List{ WebLink{"foo", 0, 3} }
                                << "" << WebLink::List{}
                                << TextSplitterPart{"foo", { WebLink{"foo", 0, 3} }};
        QTest::newRow("join1") << "foo" << WebLink::List{ WebLink{"foo", 0, 3} }
                               << "bar" << WebLink::List{ WebLink{"bar", 0, 3} }
                               << TextSplitterPart{"foo bar", { WebLink{"foo", 0, 3}, WebLink{"bar", 4, 7} }};
        QTest::newRow("join2") << "foo " << WebLink::List{ WebLink{"foo", 0, 3} }
                               << "bar" << WebLink::List{ WebLink{"bar", 0, 3} }
                               << TextSplitterPart{"foo bar", { WebLink{"foo", 0, 3}, WebLink{"bar", 4, 7} }};
        QTest::newRow("join3") << "foo" << WebLink::List{ WebLink{"foo", 0, 3} }
                               << " bar" << WebLink::List{ WebLink{"bar", 1, 4} }
                               << TextSplitterPart{"foo bar", { WebLink{"foo", 0, 3}, WebLink{"bar", 4, 7} }};
    }

    void joinText()
    {
        QFETCH(QString, text1);
        QFETCH(WebLink::List, links1);
        QFETCH(QString, text2);
        QFETCH(WebLink::List, links2);
        QFETCH(TextSplitterPart, output);

        const auto part = mTextSplitter.joinText(text1, links1, text2, links2);
        QCOMPARE(part.getText(), output.getText());
        QCOMPARE(part.getEmbeddedLinks(), output.getEmbeddedLinks());
    }

private:
    QStringList toStringList(const TextSplitterPart::List& parts)
    {
        QStringList list;

        for (const auto& part : parts)
            list.push_back(part.getText());

        return list;
    }

    TextSplitter mTextSplitter;
};
