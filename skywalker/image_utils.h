// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ImageUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool extractingText READ isExtractingText NOTIFY extractingTextChanged FINAL)
    QML_ELEMENT

public:
    explicit ImageUtils(QObject* parent = nullptr);

    bool isExtractingText() const { return mExtractingText; }
    void setExtractingText(bool extractingText);
    Q_INVOKABLE bool extractText(const QString& imgSource);

signals:
    void extractTextOk(QString source, QString text);
    void extractTextFailed(QString source, QString error);
    void extractingTextChanged();

private:
    void handleExtractTextOk(const QString& source, const QString& text);
    void handleExtractTextFailed(const QString& source, const QString& error);

    bool mExtractingText = false;
    QString mExtractingTextFromSource;
};

}
