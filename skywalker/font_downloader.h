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
    static void downloadEmojiFont();
    static float getFontScale();
    static QFont getEmojiFont();
};

}
