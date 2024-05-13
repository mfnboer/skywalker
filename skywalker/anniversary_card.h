// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QColor>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class AnniversaryCard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int years READ getYears WRITE setYears NOTIFY yearsChanged FINAL)
    Q_PROPERTY(QString imageSource READ getImageSource NOTIFY imageSourceChanged FINAL)
    Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged FINAL)
    Q_PROPERTY(QColor logoColor READ getLogoColor WRITE setLogoColor NOTIFY logoColorChanged FINAL)
    Q_PROPERTY(QColor ageColor READ getAgeColor WRITE setAgeColor NOTIFY ageColorChanged FINAL)
    QML_ELEMENT

public:
    explicit AnniversaryCard(QObject* parent = nullptr);

    int getYears() const { return mYears; }
    void setYears(int years);
    const QString& getImageSource() const { return mImageSource; }
    const QColor& getBackgroundColor() const { return mBackgroundColor; }
    void setBackgroundColor(const QColor& color);
    const QColor& getLogoColor() const { return mLogoColor; }
    void setLogoColor(const QColor& color);
    const QColor& getAgeColor() const { return mAgeColor; }
    void setAgeColor(const QColor& color);

    Q_INVOKABLE void dropCard();

signals:
    void yearsChanged();
    void imageSourceChanged();
    void backgroundColorChanged();
    void logoColorChanged();
    void ageColorChanged();

private:
    void initCard();

    int mYears = 1;
    QString mImageSource;
    QColor mBackgroundColor{"pink"};
    QColor mLogoColor{10,122,255};
    QColor mAgeColor{"white"};
};

}
