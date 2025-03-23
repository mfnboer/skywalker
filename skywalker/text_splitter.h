// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "web_link.h"
#include <qqmlintegration.h>
#include <QObject>

namespace Skywalker {

class TextSplitterPart
{
    Q_GADGET
    Q_PROPERTY(QString text READ getText CONSTANT FINAL)
    Q_PROPERTY(WebLink::List embeddedLinks READ getEmbeddedLinks CONSTANT FINAL)

public:
    using List = QList<TextSplitterPart>;

    TextSplitterPart() = default;
    explicit TextSplitterPart(const QString& text, const WebLink::List& embeddedLinks);

    bool operator==(const TextSplitterPart&) const = default;

    const QString& getText() const { return mText; }
    const WebLink::List& getEmbeddedLinks() const { return mEmbeddedLinks; }

private:
    QString mText;
    WebLink::List mEmbeddedLinks;
};

class TextSplitter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TextSplitter(QObject* parent = nullptr);

    Q_INVOKABLE TextSplitterPart::List splitText(
        const QString& text, const WebLink::List& embeddedLinks, int maxLength,
        int minSplitLineLength, int maxParts = 1000000);
};

}
