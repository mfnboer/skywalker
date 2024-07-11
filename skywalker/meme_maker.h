// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "shared_image_provider.h"
#include <QImage>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class MemeMaker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString memeImgSource READ getMemeImgSource NOTIFY memeImgSourceChanged FINAL)
    Q_PROPERTY(QString topText READ getTopText WRITE setTopText NOTIFY topTextChanged FINAL)
    Q_PROPERTY(QString bottomText READ getBottomText WRITE setBottomText NOTIFY bottomTextChanged FINAL)
    QML_ELEMENT

public:
    explicit MemeMaker(QObject* parent = nullptr);

    Q_INVOKABLE bool setOrigImage(const QString& imgSource);

    QString getMemeImgSource() const;
    void setMemeImgSource(const QString& source, SharedImageProvider* provider);
    Q_INVOKABLE void releaseMemeOwnership();

    const QString& getTopText() const { return mTopText; }
    void setTopText(const QString& text);
    const QString& getBottomText() const { return mBottomText; }
    void setBottomText(const QString& text);

signals:
    void memeImgSourceChanged();
    void topTextChanged();
    void bottomTextChanged();

private:
    double fontSizeRatio() const;
    int marginSize() const;
    QPainterPath createTextPath(int x, int y, const QString& text, int maxWidth, int& fontPx) const;
    std::vector<QPainterPath> createTextMultiPathList(int x, int y, const QString& text, int maxWidth, int pathCount, int& fontPx) const;
    std::vector<QPainterPath> createTextPathList(int x, int y, const QString& text, int maxWidth, int& fontPx) const;
    void moveToBottom(std::vector<QPainterPath>& paths) const;
    void center(int maxWidth, QPainterPath& path) const;
    void addText();

    QImage mOrigImage;
    SharedImageSource::Ptr mMemeImgSource;
    QString mTopText;
    QString mBottomText;
};

}
