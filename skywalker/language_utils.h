// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QtQmlIntegration>
#include <QObject>

namespace Skywalker {

class Language : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString code MEMBER mCode CONSTANT FINAL)
    Q_PROPERTY(QString shortCode MEMBER mShortCode CONSTANT FINAL)
    Q_PROPERTY(QString nativeName MEMBER mNativeName CONSTANT FINAL)
    QML_ELEMENT

public:
    explicit Language(QObject* parent = nullptr);
    Language(const QString& code, const QString& nativeName, QObject* parent = nullptr);

    const QString& getCode() const { return mCode; }
    const QString& getShortCode() const { return mShortCode; }
    const QString& getNativeName() const { return mNativeName; }

private:
    QString mCode;
    QString mShortCode;
    QString mNativeName;
};

class LanguageUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<Language*> languages MEMBER mLanguages CONSTANT FINAL)
    QML_ELEMENT

public:
    explicit LanguageUtils(QObject* parent = nullptr);

private:
    void initLanguages();

    QList<Language*> mLanguages;
};

}
