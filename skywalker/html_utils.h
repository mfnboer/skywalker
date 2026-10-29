// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QRegularExpression>

namespace Skywalker {

class HtmlUtils
{
public:
    using Attrs = std::unordered_map<QString, QString>; // attr -> value
    using ElementAttrs = std::unordered_map<QString, Attrs>; // element -> value

    static QStringList extractIconLinkElements(const QByteArray& data);
    static QStringList extractImgElements(const QByteArray& data);

    static Attrs getAttributes(const QString& element);
    static ElementAttrs getAttributes(const QStringList& elements);

    static QString getAttrValue(const Attrs& attrs, const QString& attr);

    static QString toElement(const QString& name, const Attrs& attrs);

    static QString scaleHtmlImgTags(const QString& text, int width);
    static QSize getWidthAndHeight(const Attrs& attrs);

private:
    static QStringList extractElements(const QByteArray& data, const QRegularExpression& re);
    static QStringList extractElements(const QByteArray& data, const std::vector<QRegularExpression>& reList);
};

}