// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "font_downloader.h"
#include "file_utils.h"
#include "temp_file_holder.h"
#include "unicode_fonts.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QTextBoundaryFinder>
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QFile>
#endif

namespace Skywalker {

QString FontDownloader::sEmojiFontFamily = "Noto Color Emoji";
QString FontDownloader::sEmojiFontSource;

QString FontDownloader::getEmojiFontFamily()
{
    return sEmojiFontFamily;
}

QFont FontDownloader::getEmojiFont()
{
    QFont font = QGuiApplication::font();
    font.setFamily(sEmojiFontFamily);
    return font;
}

void FontDownloader::initAppFonts()
{
    UserSettings userSettings;

    addApplicationFonts();
    downloadEmojiFont();

    QFont font = QGuiApplication::font();
    const float fontScale = getFontScale();
    auto fontFamilies = font.families();
    fontFamilies.push_back(sEmojiFontFamily);
    font.setFamilies(fontFamilies);
    font.setWeight(QFont::Weight(350));
    font.setPixelSize(std::roundf(16 * fontScale));
    QGuiApplication::setFont(font);

    qInfo() << "Font:" << font;
    qInfo() << "Font pt size:" << font.pointSize();
    qInfo() << "Font px size:" << font.pixelSize();
    qInfo() << "Font families:" << font.families();
    qInfo() << "Font family:" << font.family();
    qInfo() << "Font default family:" << font.defaultFamily();
    qInfo() << "Font style hint:" << font.styleHint();
    qInfo() << "Font style strategy:" << font.styleStrategy();
    qInfo() << "Font scale:" << fontScale;
    qInfo() << "Font family emoji:" << getEmojiFontFamily();
    qInfo() << "Font emoji source:" << getEmojiFontSource();

    ATProto::RichTextMaster::setHtmlCleanup([](const QString& s){ return UnicodeFonts::setEmojiFontCombinedEmojis(s); });
}

void FontDownloader::addFont(const QString& fontFileName)
{
    const int fontId = QFontDatabase::addApplicationFont(fontFileName);
    qDebug() << fontFileName << "fontId:" << fontId;

    if (fontId >= 0)
        qInfo() << "FONT FAMILIES:" << QFontDatabase::applicationFontFamilies(fontId);
    else
        qWarning() << "Failed to add:" << fontFileName;
}

void FontDownloader::addApplicationFonts()
{
    // The Noto Sans Math font has the math symbols, i.e. the bold, italic, wide unicode
    // characters that people often use in posts.
    addFont(QStringLiteral(":/fonts/NotoSansMath-Regular.ttf"));
    addFont(QStringLiteral(":/fonts/unicode.impact.ttf"));
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

    const QByteArray fileData = file.readAll();
    qDebug() << "Data size:" << fileData.size();
    const int fontId = QFontDatabase::addApplicationFontFromData(fileData);
    qInfo() << "Font added ID:" << fontId;

    if (fontId >= 0)
    {
        const auto fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        qInfo() << "Emoji font families:" << fontFamilies;

        if (!fontFamilies.empty())
        {
            sEmojiFontFamily = fontFamilies[0];
            qInfo() << "Set emoji font family:" << sEmojiFontFamily;
        }
        else
        {
            qWarning() << "No emoji font families found:" << fontId;
        }
    }
    else
    {
        qWarning() << "Failed to add emoji font:" << fontId;
    }
#endif
}

void FontDownloader::downloadEmojiFontFile()
{
#ifdef Q_OS_ANDROID
    sEmojiFontSource.clear();

    auto fd = QJniObject::callStaticMethod<jint>("com/gmail/mfnboer/GMSEmojiFontDownloader",
                                                 "getFontFileDescriptor",
                                                 "()I");

    if (fd < 0)
    {
        qWarning() << "Failed to get file descriptor to download emoji font";
        return;
    }

    auto fontFile = FileUtils::createTempFile(fd, "ttf");
    sEmojiFontSource = "file://" + fontFile->fileName();
    TempFileHolder::instance().put(std::move(fontFile));

    qInfo() << "Created emoji font file:" << sEmojiFontSource;
#endif
}

void FontDownloader::installBundleEmojiFontSource()
{
    sEmojiFontSource = "qrc:/fonts/NotoColorEmoji.ttf";
}

float FontDownloader::getFontScale()
{
#ifdef Q_OS_ANDROID
    auto fontScale = QJniObject::callStaticMethod<jfloat>("com/gmail/mfnboer/FontUtils",
                                                         "getFontScale",
                                                         "()F");

    qDebug() << "Font scale:" << fontScale;
    return fontScale;
#else
    return 1.0;
#endif
}

}
