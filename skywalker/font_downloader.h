// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QFont>

namespace Skywalker {

class FontDownloader
{
public:
    static void initAppFonts();
    static void addApplicationFonts();

    // Download and install font.
    static void downloadEmojiFont();

    // Download and create a temp ttf file.
    static void downloadEmojiFontFile();

    static float getFontScale();
    static QString getEmojiFontFamily();
    static QFont getEmojiFont();
    static QString getEmojiFontFile() { return sEmojiFontFile; }

private:
    static void addFont(const QString& fontFileName);

    static QString sEmojiFontFamily;
    static QString sEmojiFontFile;
};

}
