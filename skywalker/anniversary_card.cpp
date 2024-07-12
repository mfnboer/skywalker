// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "anniversary_card.h"
#include "definitions.h"
#include "shared_image_provider.h"
#include "utils.h"
#include <QByteArray>
#include <QPainter>
#include <QSvgRenderer>

namespace Skywalker {

static constexpr char const* CARD_SVG = R"#(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 320 286"><path fill="rgb(%1,%2,%3)" d="M69.364 19.146c36.687 27.806 76.147 84.186 90.636 114.439 14.489-30.253 53.948-86.633 90.636-114.439C277.107-.917 320-16.44 320 32.957c0 9.865-5.603 82.875-8.889 94.729-11.423 41.208-53.045 51.719-90.071 45.357 64.719 11.12 81.182 47.953 45.627 84.785-80 82.874-106.667-44.333-106.667-44.333s-26.667 127.207-106.667 44.333c-35.555-36.832-19.092-73.665 45.627-84.785-37.026 6.362-78.648-4.149-90.071-45.357C5.603 115.832 0 42.822 0 32.957 0-16.44 42.893-.917 69.364 19.147Z"/></svg>)#";
static constexpr int CARD_MARGIN = 50;
static constexpr int CARD_WIDTH = 320 * 2 + CARD_MARGIN * 2;
static constexpr int CARD_HEIGHT = 286 * 2 + CARD_MARGIN * 2;

AnniversaryCard::AnniversaryCard(QObject* parent) :
    QObject(parent)
{
    initCard();
}

void AnniversaryCard::setYears(int years)
{
    if (years != mYears)
    {
        mYears = years;
        initCard();
        emit yearsChanged();
    }
}

void AnniversaryCard::setBackgroundColor(const QColor& color)
{
    if (color != mBackgroundColor)
    {
        mBackgroundColor = color;
        initCard();
        emit backgroundColorChanged();
    }
}

void AnniversaryCard::setLogoColor(const QColor& color)
{
    if (color != mLogoColor)
    {
        mLogoColor = color;
        initCard();
        emit logoColorChanged();
    }
}

void AnniversaryCard::setAgeColor(const QColor& color)
{
    if (color != mAgeColor)
    {
        mAgeColor = color;
        initCard();
        emit ageColorChanged();
    }
}

void AnniversaryCard::dropCard()
{
    if (mImageSource.isEmpty())
        return;

    auto* imageProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    imageProvider->removeImage(mImageSource);
}

void AnniversaryCard::initCard()
{
    QImage cardImage(CARD_WIDTH, CARD_HEIGHT, QImage::Format_ARGB32);
    cardImage.fill(mBackgroundColor);
    QPainter painter;

    if (!painter.begin(&cardImage))
    {
        qWarning() << "Cannot paint on image";
        return;
    }

    painter.setViewport(cardImage.rect().adjusted(CARD_MARGIN, CARD_MARGIN, -CARD_MARGIN, -CARD_MARGIN));
    const QString svg = QString(CARD_SVG).arg(mLogoColor.red()).arg(mLogoColor.green()).arg(mLogoColor.blue());
    qDebug() << svg;
    QSvgRenderer svgRenderer(svg.toUtf8());
    qDebug() << "SVG valid:" << svgRenderer.isValid();
    svgRenderer.render(&painter);

    painter.setPen(QPen(mAgeColor));
    painter.setFont(QFont("Impact", 128, QFont::Black));
    painter.drawText(cardImage.rect().adjusted(0, 130, 0, 0), Qt::AlignCenter,
                     QString::number(mYears));

    painter.setViewport(cardImage.rect());
    painter.setPen(QPen(Utils::determineForegroundColor(mBackgroundColor, "black", "white")));
    QFont font;
    font.setPointSize(18);
    font.setItalic(true);
    painter.setFont(font);
    painter.drawText(cardImage.rect().adjusted(0, 0, -10, -5), Qt::AlignRight | Qt::AlignBottom,
                     QString("sent with %1").arg(SKYWALKER_HANDLE));
    painter.end();

    auto* imageProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);

    if (!mImageSource.isEmpty())
        imageProvider->removeImage(mImageSource);

    mImageSource = imageProvider->addImage(cardImage);
    emit imageSourceChanged();
}

}
