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
        QTest::addColumn<NamedLink::List>("links");
        QTest::addColumn<TextSplitterPart::List>("output");

        QTest::newRow("split 1") << "aaaa bbbb cccc dddd" << 10
            << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "bbbb cccc", 5, 14}, NamedLink{QEnums::LINK_TYPE_WEB, "dddd", 15, 19} }
            << TextSplitterPart::List{
                    TextSplitterPart{ "aaaa ", {} },
                    TextSplitterPart{ "bbbb cccc ", { NamedLink{QEnums::LINK_TYPE_WEB, "bbbb cccc", 0, 9} } },
                    TextSplitterPart{ "dddd", { NamedLink{QEnums::LINK_TYPE_WEB, "dddd", 0, 4} } }};

        QTest::newRow("split 2") << "aaaa bbbb cccc dddd" << 10
            << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "dddd", 15, 19} }
            << TextSplitterPart::List{
                    TextSplitterPart{ "aaaa bbbb ", {} },
                    TextSplitterPart{ "cccc dddd", { NamedLink{QEnums::LINK_TYPE_WEB, "dddd", 5, 9} } }};
    }

    void splitTextWithLinks()
    {
        QFETCH(QString, text);
        QFETCH(int, maxLength);
        QFETCH(NamedLink::List, links);
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
        QTest::addColumn<NamedLink::List>("links1");
        QTest::addColumn<QString>("text2");
        QTest::addColumn<NamedLink::List>("links2");
        QTest::addColumn<TextSplitterPart>("output");

        QTest::newRow("empty") << "" << NamedLink::List{} << "" << NamedLink::List{} << TextSplitterPart{"", {}};
        QTest::newRow("empty1") << "" << NamedLink::List{}
                                << "foo" << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3} }
                                << TextSplitterPart{"foo", { NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3} }};
        QTest::newRow("empty2") << "foo" << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3} }
                                << "" << NamedLink::List{}
                                << TextSplitterPart{"foo", { NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3} }};
        QTest::newRow("join1") << "foo" << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3} }
                               << "bar" << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "bar", 0, 3} }
                               << TextSplitterPart{"foo bar", { NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3}, NamedLink{QEnums::LINK_TYPE_WEB, "bar", 4, 7} }};
        QTest::newRow("join2") << "foo " << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3} }
                               << "bar" << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "bar", 0, 3} }
                               << TextSplitterPart{"foo bar", { NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3}, NamedLink{QEnums::LINK_TYPE_WEB, "bar", 4, 7} }};
        QTest::newRow("join3") << "foo" << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3} }
                               << " bar" << NamedLink::List{ NamedLink{QEnums::LINK_TYPE_WEB, "bar", 1, 4} }
                               << TextSplitterPart{"foo bar", { NamedLink{QEnums::LINK_TYPE_WEB, "foo", 0, 3}, NamedLink{QEnums::LINK_TYPE_WEB, "bar", 4, 7} }};
    }

    void joinText()
    {
        QFETCH(QString, text1);
        QFETCH(NamedLink::List, links1);
        QFETCH(QString, text2);
        QFETCH(NamedLink::List, links2);
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
