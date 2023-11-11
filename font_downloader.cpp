// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "font_downloader.h"
#include <QFontDatabase>
#include <QGuiApplication>
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QFile>
#endif

namespace Skywalker {

void FontDownloader::initAppFonts()
{
    addApplicationFonts();
    downloadEmojiFont();

    QFont font = QGuiApplication::font();
    auto fontFamilies = font.families();
    fontFamilies.push_back("Noto Color Emoji");
    font.setFamilies(fontFamilies);
    font.setWeight(QFont::Weight(350));
    font.setPixelSize(16);
    QGuiApplication::setFont(font);

    qDebug() << "Font:" << font;
    qDebug() << "Font pt size:" << font.pointSize();
    qDebug() << "Font px size:" << font.pixelSize();
    qDebug() << "Font families:" << font.families();
    qDebug() << "Font family:" << font.family();
    qDebug() << "Font default family:" << font.defaultFamily();
    qDebug() << "Font style hint:" << font.styleHint();
}

void FontDownloader::addApplicationFonts()
{
    // The Noto Sans Math font has the math symbols, i.e. the bold, italic, wide unicode
    // characters that people often use in posts.
    const int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansMath-Regular.ttf"));
    qDebug() << "NotoSansMath font:" << fontId;

    if (fontId >= 0)
        qDebug() << "FONT FAMILIES:" << QFontDatabase::applicationFontFamilies(fontId);
    else
        qWarning() << "Failed to add NotoSansMath-Regular.ttf";
}

void FontDownloader::downloadEmojiFont()
{
#ifdef Q_OS_ANDROID
    auto fd = QJniObject::callStaticMethod<jint>("com/gmail/mfnboer/GMSEmojiFontDownloader",
                                                 "getFontFileDescriptor",
                                                 "()I");

    if (fd < 0)
    {
        qWarning() << "Failed to get file descriptor to download emoji font";
        return;
    }

    QFile file;
    file.open(fd, QFile::OpenModeFlag::ReadOnly, QFile::FileHandleFlag::AutoCloseHandle);

    const int fontId = QFontDatabase::addApplicationFontFromData(file.readAll());
    qDebug() << "Font added ID:" << fontId;

    if (fontId >= 0)
        qDebug() << "FONT FAMILIES:" << QFontDatabase::applicationFontFamilies(fontId);
#endif
}

}
