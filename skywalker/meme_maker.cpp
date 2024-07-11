// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "meme_maker.h"
#include "photo_picker.h"
#include "unicode_fonts.h"
#include <QFont>
#include <QPainter>
#include <QPainterPath>

namespace Skywalker {

constexpr int FONT_MAX_PX = 38;
constexpr int FONT_MIN_PX = 24;
constexpr int MARGIN = 10;
constexpr int MAX_TEXT_PATHS = 4;

// This is the width on a Galaxy S22. Scaling font size to this width
// makes the text size independent from the device creating the meme and
// gives a decent text size on various devices.
constexpr int MOBILE_SCREEN_WIDTH = 340;

MemeMaker::MemeMaker(QObject* parent) :
    QObject(parent)
{}

bool MemeMaker::setOrigImage(const QString& imgSource)
{
    mOrigImage = PhotoPicker::loadImage(imgSource);

    if (mOrigImage.isNull())
        return false;

    auto* imageProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    const QString memeSource = imageProvider->addImage(mOrigImage);
    setMemeImgSource(memeSource, imageProvider);
    setTopText("");
    setBottomText("");
    return true;
}

QString MemeMaker::getMemeImgSource() const
{
    return mMemeImgSource ? mMemeImgSource->getSource() : "";
}

void MemeMaker::setMemeImgSource(const QString& source, SharedImageProvider* provider)
{
    mMemeImgSource = std::make_unique<SharedImageSource>(source, provider);
    emit memeImgSourceChanged();
}

void MemeMaker::releaseMemeOwnership()
{
    mMemeImgSource->clear();
}

void MemeMaker::setTopText(const QString& text)
{
    if (text == mTopText)
        return;

    mTopText = text;
    addText();
    emit topTextChanged();
}

void MemeMaker::setBottomText(const QString& text)
{
    if (text == mBottomText)
        return;

    mBottomText = text;
    addText();
    emit bottomTextChanged();
}

double MemeMaker::fontSizeRatio() const
{
    return mOrigImage.width() / (float)MOBILE_SCREEN_WIDTH;
}

int MemeMaker::marginSize() const
{
    return MARGIN * fontSizeRatio();
}

void MemeMaker::center(int maxWidth, QPainterPath& path) const
{
    const int dx = std::max((maxWidth - (int)path.boundingRect().width()), 0) / 2.0;
    path.translate(dx, 0);
}

QPainterPath MemeMaker::createTextPath(int x, int y, const QString& text, int maxWidth, int& fontPx) const
{
    QFont font;
    font.setFamily("Impact");
    font.setPixelSize(fontPx * fontSizeRatio());
    font.setWeight(QFont::Black);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addText(x, y + font.pixelSize(), font, text);

    while (path.boundingRect().width() > maxWidth && fontPx > FONT_MIN_PX)
    {
        --fontPx;
        font.setPixelSize(fontPx * fontSizeRatio());
        path.clear();
        path.addText(x, y + font.pixelSize(), font, text);
    }

    center(maxWidth, path);
    return path;
}

std::vector<QPainterPath> MemeMaker::createTextMultiPathList(int x, int y, const QString& text, int maxWidth, int pathCount, int& fontPx) const
{
    std::vector<QPainterPath> paths;
    const int maxLength = text.length() / pathCount + 1;
    const auto textList = UnicodeFonts::splitText(text, maxLength, 1, pathCount);
    int lineY = y;
    int firstLineFontPx = 0;
    int smallestFontPx = fontPx;

    for (const auto& line : textList)
    {
        const auto linePath = createTextPath(x, lineY, line, maxWidth, fontPx);
        paths.push_back(linePath);
        lineY += linePath.boundingRect().height();

        if (firstLineFontPx == 0)
        {
            firstLineFontPx = fontPx;
            smallestFontPx = fontPx;
        }
        else if (fontPx < smallestFontPx)
        {
            smallestFontPx = fontPx;
        }
    }

    if (smallestFontPx < firstLineFontPx)
    {
        paths.clear();
        lineY = y;
        fontPx = smallestFontPx;

        for (const auto& line : textList)
        {
            const auto linePath = createTextPath(x, lineY, line, maxWidth, fontPx);
            paths.push_back(linePath);
            lineY += linePath.boundingRect().height();
        }
    }

    return paths;
}

std::vector<QPainterPath> MemeMaker::createTextPathList(int x, int y, const QString& text, int maxWidth, int& fontPx) const
{
    const int maxFontPx = fontPx;
    std::vector<QPainterPath> paths;
    const auto path = createTextPath(x, y, text, maxWidth, fontPx);

    if (path.boundingRect().width() <= maxWidth)
    {
        paths.push_back(path);
        return paths;
    }

    int pathCount = 2;
    bool done = false;

    while (!done && pathCount <= MAX_TEXT_PATHS)
    {
        fontPx = maxFontPx;
        paths = createTextMultiPathList(x, y, text, maxWidth, pathCount, fontPx);
        done = true;

        for (const auto& p : paths)
        {
            if (p.boundingRect().width() > maxWidth)
            {
                ++pathCount;
                done = false;
                break;
            }
        }
    }

    return paths;
}

static int calcHeight(const std::vector<QPainterPath>& paths)
{
    int height = 0;

    for (const auto& p : paths)
        height += p.boundingRect().height();

    return height;
}

void MemeMaker::moveToBottom(std::vector<QPainterPath>& paths) const
{
    const int height = calcHeight(paths);
    const int dy = mOrigImage.height() - height - marginSize();

    for (auto& p : paths)
        p.translate(0, dy);
}

void MemeMaker::addText()
{
    const int x = marginSize();
    const int maxWidth = mOrigImage.width() - 2 * marginSize();
    int topFontPx = FONT_MAX_PX;
    auto topPaths = createTextPathList(x, 0, mTopText, maxWidth, topFontPx);
    int bottomFontPx = topFontPx;
    auto bottomPaths = createTextPathList(x, 0, mBottomText, maxWidth, bottomFontPx);

    // Make the font size of top and bottom text equal for esthetics.
    if (!topPaths.empty() && !bottomPaths.empty() && bottomFontPx < topFontPx)
    {
        topFontPx = bottomFontPx;
        topPaths = createTextPathList(x, 0, mTopText, maxWidth, topFontPx);
    }

    moveToBottom(bottomPaths);

    QImage memeImage = mOrigImage;
    QPainter painter;

    if (!painter.begin(&memeImage))
    {
        qWarning() << "Cannot paint on image";
        return;
    }

    QPen pen(Qt::black);
    pen.setWidth(std::max(1.0, fontSizeRatio()));
    painter.setPen(pen);
    painter.setBrush(Qt::white);

    for (const auto& path : topPaths)
        painter.drawPath(path);

    for (const auto& path : bottomPaths)
        painter.drawPath(path);

    painter.end();

    auto* provider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    const QString source = provider->addImage(memeImage);
    setMemeImgSource(source, provider);
}

}
