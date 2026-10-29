// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "html_utils.h"
#include <QSize>

namespace Skywalker {

static std::vector<QRegularExpression> quoteVariants(const QString& reStr)
{
    return std::vector<QRegularExpression>{
        QRegularExpression(reStr.arg('"')),
        QRegularExpression(reStr.arg('\''))
    };
}

QStringList HtmlUtils::extractIconLinkElements(const QByteArray& data)
{
    static const QString iconLinkReStr(R"(<link[^>]*rel=%1[^%1]*icon[^%1]*%1[^>]*>)");
    static const auto iconLinkREs = quoteVariants(iconLinkReStr);

    return extractElements(data, iconLinkREs);
}

QStringList HtmlUtils::extractImgElements(const QByteArray& data)
{
    static const QString imgReStr(R"(<img[^>*]src=%1[^%1]*%1[^>]*>)");
    static const auto imgREs = quoteVariants(imgReStr);

    return extractElements(data, imgREs);
}

QStringList HtmlUtils::extractElements(const QByteArray& data, const QRegularExpression& re)
{
    QStringList elements;

    for (const auto& match : re.globalMatch(data))
    {
        if (!match.hasMatch())
            continue;

        const QString& element = match.captured();
        qDebug() << "Element:" << element;
        elements.push_back(element);
    }

    return elements;
}

QStringList HtmlUtils::extractElements(const QByteArray& data, const std::vector<QRegularExpression>& reList)
{
    QStringList elements;

    for (const auto& re : reList)
        elements.append(extractElements(data, re));

    return elements;
}

HtmlUtils::Attrs HtmlUtils::getAttributes(const QString& element)
{
    static const QString attrReStr(R"(([a-zA-Z_:][-a-zA-Z0-9_:.]*)\s*=\s*%1([^%1]*)%1)");
    static const auto attrREs = quoteVariants(attrReStr);

    Attrs attrs;

    for (const auto& attrRE : attrREs)
    {
        for (const auto& match : attrRE.globalMatch(element))
        {
            if (!match.hasMatch())
                continue;

            const QString attr = match.captured(1).trimmed().toLower();
            QString value = match.captured(2).trimmed();
            qDebug() << attr << "=" << value;
            attrs[attr] = value;
        }
    }

    return attrs;
}

HtmlUtils::ElementAttrs HtmlUtils::getAttributes(const QStringList& elements)
{
    ElementAttrs elementAttrs;

    for (const auto& elem : elements)
    {
        qDebug() << "get attrs from:" << elem;
        auto& attrs = elementAttrs[elem];
        attrs = getAttributes(elem);
    }

    return elementAttrs;
}

QString HtmlUtils::getAttrValue(const Attrs& attrs, const QString& attr)
{
    return attrs.contains(attr) ? attrs.at(attr) : "";
}

QString HtmlUtils::toElement(const QString& name, const Attrs& attrs)
{
    QString element = QString("<%1").arg(name);

    for (const auto& [key, value] : attrs)
        element += QString(" %1=\"%2\"").arg(key, value);

    element += ">";
    return element;
}

// The scaling is not great, but good enough for its current use (posts bridged from
// Mastodon)
QString HtmlUtils::scaleHtmlImgTags(const QString& text, int width)
{
    qDebug() << "Scale img tag to width:" << width << "text:" << text.chopped(40);
    QString newText = text;
    const auto imgElems = extractImgElements(text.toUtf8());
    auto imgAttrs = getAttributes(imgElems);

    for (auto& [img, attrs] : imgAttrs)
    {
        qDebug() << "Img:" << img;
        const QSize size = getWidthAndHeight(attrs);

        if (!size.isValid())
        {
            if (attrs.contains("style"))
                attrs["style"] = "max-width: 100%; height: auto;";
            else
                qWarning() << "Cannot scale image:" << img;
        }
        else if (size.width() > width)
        {
            int newHeight = ((double)size.height() / size.width()) * width;
            qDebug() << "imgWidth:" << size.width() << "imgHeight:" << size.height() << "width:" << width << "height:" << newHeight;
            attrs["width"] = QString::number(width);
            attrs["height"] = QString::number(newHeight);
        }

        const auto scaledImg = toElement("img", attrs);
        qDebug() << "Scaled img:" << scaledImg;
        newText.replace(img, scaledImg);
    }

    return newText;
}

QSize HtmlUtils::getWidthAndHeight(const Attrs& attrs)
{
    QString widthStr = getAttrValue(attrs, "width");
    QString heightStr = getAttrValue(attrs, "height");

    if (widthStr.isEmpty() || heightStr.isEmpty())
        return {};

    int width = widthStr.toInt();
    int height = heightStr.toInt();

    if (width <= 0 || height <=0)
        return {};

    return QSize(width, height);
}

}