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
    void uniqueEmoji_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<QStringList>("output");

        QTest::newRow("empty") << "" << QStringList{};
        QTest::newRow("none") << "hello world" << QStringList{};
        QTest::newRow("grinning face") << "😀" << QStringList{"😀"};
        QTest::newRow("duplicate grinning face") << "😀😀" << QStringList{"😀"};
        QTest::newRow("hello grinning face") << "hello 😀 world" << QStringList{"😀"};
        QTest::newRow("hello grinning face star face") << "hello 😀 world 🤩" << QStringList{"😀","🤩"};
        QTest::newRow("rainbow flag") << "😀🏳️‍🌈🤩🏳️‍🌈" << QStringList{"🏳️‍🌈","😀","🤩"};
        QTest::newRow("lifting weights medium light skin") << "🏋🏼" << QStringList{"🏋🏼"};
        QTest::newRow("lifting weights skins") << "🏋🏻🏋🏼🏋🏿" << QStringList{"🏋🏻","🏋🏼","🏋🏿"};
    }

    void uniqueEmoji()
    {
        QFETCH(QString, text);
        QFETCH(QStringList, output);
        QCOMPARE(UnicodeFonts::getUniqueEmojis(text), output);
    }
};

