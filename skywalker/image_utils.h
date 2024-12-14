// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ImageUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool installing READ isInstalling NOTIFY installingChanged FINAL)
    Q_PROPERTY(bool extractingText READ isExtractingText NOTIFY extractingTextChanged FINAL)
    QML_ELEMENT

public:
    explicit ImageUtils(QObject* parent = nullptr);

    bool isInstalling() const { return mInstalling; }
    void setInstalling(bool installing);
    bool isExtractingText() const { return mExtractingText; }
    void setExtractingText(bool extractingText);

    // NOTE: not used as checking for availability triggers automatic
    // installation when the module is not installed. That is rather
    // useless.
    Q_INVOKABLE void checkAvailability(QEnums::Script script);

    Q_INVOKABLE void installModule(QEnums::Script script);
    Q_INVOKABLE bool extractText(QEnums::Script script, const QString& imgSource);

signals:
    void checkAvailabilityOk(QEnums::Script script, bool available);
    void checkAvailabilityFailed(QEnums::Script script, QString error);
    void installModuleProgress(QEnums::Script script, int progressPercentage);
    void installModuleOk(QEnums::Script script);
    void installModuleFailed(QEnums::Script script, QString error);
    void extractTextOk(QString source, QString text);
    void extractTextFailed(QString source, QString error);
    void installingChanged();
    void extractingTextChanged();

private:
    void handleCheckAvailabilityOk(QEnums::Script script, bool available);
    void handleCheckAvailabilityFailed(QEnums::Script script, QString error);
    void handleInstallModuleProgress(QEnums::Script script, int progressPercentage);
    void handleInstallModuleOk(QEnums::Script script);
    void handleInstallModuleFailed(QEnums::Script script, QString error);
    void handleExtractTextOk(const QString& source, const QString& text);
    void handleExtractTextFailed(const QString& source, const QString& error);

    bool mInstalling = false;
    bool mExtractingText = false;
    QString mExtractingTextFromSource;
};

}
