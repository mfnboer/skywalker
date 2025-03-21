// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <text_differ.h>
#include <QtTest/QTest>

using namespace Skywalker;

class TestTextDiffer : public QObject
{
    Q_OBJECT
private slots:
    void diff_data()
    {
        QTest::addColumn<QString>("oldText");
        QTest::addColumn<QString>("newText");
        QTest::addColumn<TextDiffer::Result>("output");
        QTest::newRow("none empty") << "" << "" << createNoneResult();
        QTest::newRow("none") << "foo" << "foo" << createNoneResult();
        QTest::newRow("insert after") << "foo" << "foobar!" << createInsertResult(3, 6);
        QTest::newRow("insert before") << "foo" << "bar!foo" << createInsertResult(0, 3);
        QTest::newRow("insert mid") << "fr" << "foobar" << createInsertResult(1, 4);
        QTest::newRow("insert empty") << "" << "foo" << createInsertResult(0, 2);
        QTest::newRow("delete after") << "foobar!" << "foo" << createDeleteResult(3, 6);
        QTest::newRow("delete before") << "foo!bar" << "bar" << createDeleteResult(0, 3);
        QTest::newRow("delete mid") << "foobar" << "fr" << createDeleteResult(1, 4);
        QTest::newRow("delete all") << "foobar" << "" << createDeleteResult(0, 5);
        QTest::newRow("replace front") << "foobar!" << "worldbar!" << createReplaceResult(0, 2, 0, 4);
        QTest::newRow("replace back") << "foobar!" << "fooworld" << createReplaceResult(3, 6, 3, 7);
        QTest::newRow("replace mid") << "foobar!" << "fooworld!" << createReplaceResult(3, 5, 3, 7);
        QTest::newRow("replace all") << "foo" << "barbar" << createReplaceResult(0, 2, 0, 5);
        QTest::newRow("insert") << "xhello link" << "xxhello link" << createInsertResult(1, 1);
    }

    void diff()
    {
        QFETCH(QString, oldText);
        QFETCH(QString, newText);
        QFETCH(TextDiffer::Result, output);
        const auto result = TextDiffer::diff(oldText, newText);
        QVERIFY2(equalResults(result, output), qPrintable(QString("result [%1] output [%2]")
                .arg(resultToString(result), resultToString(output))));
    }

private:
    QString resultToString(const TextDiffer::Result& result)
    {
        return QString("type: %1 new: %2 - %3 old: %4 - %5")
            .arg((int)result.mType)
            .arg(result.mNewStartIndex).arg(result.mNewEndIndex)
            .arg(result.mOldStartIndex).arg(result.mOldEndIndex);
    }

    bool equalResults(const TextDiffer::Result& lhs, const TextDiffer::Result& rhs)
    {
        if (lhs.mType != rhs.mType)
            return false;

        switch (lhs.mType)
        {
        case TextDiffType::NONE:
            return true;
        case TextDiffType::INSERTED:
            return lhs.mNewStartIndex == rhs.mNewStartIndex && lhs.mNewEndIndex == rhs.mNewEndIndex;
        case TextDiffType::DELETED:
            return lhs.mOldStartIndex == rhs.mOldStartIndex && lhs.mOldEndIndex == rhs.mOldEndIndex;
        case TextDiffType::REPLACED:
            return lhs.mNewStartIndex == rhs.mNewStartIndex && lhs.mNewEndIndex == rhs.mNewEndIndex &&
                    lhs.mOldStartIndex == rhs.mOldStartIndex && lhs.mOldEndIndex == rhs.mOldEndIndex;
        }

        return false;
    }

    TextDiffer::Result createNoneResult() const
    {
        TextDiffer::Result result;
        result.mType = TextDiffType::NONE;
        return result;
    }

    TextDiffer::Result createInsertResult(int startIndex, int endIndex) const
    {
        TextDiffer::Result result;
        result.mType = TextDiffType::INSERTED;
        result.mNewStartIndex = startIndex;
        result.mNewEndIndex = endIndex;
        return result;
    }

    TextDiffer::Result createDeleteResult(int startIndex, int endIndex) const
    {
        TextDiffer::Result result;
        result.mType = TextDiffType::DELETED;
        result.mOldStartIndex = startIndex;
        result.mOldEndIndex = endIndex;
        return result;
    }

    TextDiffer::Result createReplaceResult(int oldStartIndex, int oldEndIndex, int newStartIndex, int newEndIndex) const
    {
        TextDiffer::Result result;
        result.mType = TextDiffType::REPLACED;
        result.mOldStartIndex = oldStartIndex;
        result.mOldEndIndex = oldEndIndex;
        result.mNewStartIndex = newStartIndex;
        result.mNewEndIndex = newEndIndex;
        return result;
    }
};
