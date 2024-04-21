// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QtQmlIntegration>
#include <QObject>

namespace Skywalker {

class Language
{
    Q_GADGET
    Q_PROPERTY(QString shortCode MEMBER mShortCode CONSTANT FINAL)
    Q_PROPERTY(QString nativeName MEMBER mNativeName CONSTANT FINAL)
    QML_VALUE_TYPE(language)

public:
    Language() = default;
    Language(const QString& code, const QString& nativeName);

    const QString& getShortCode() const { return mShortCode; }
    const QString& getNativeName() const { return mNativeName; }

private:
    QString mShortCode;
    QString mNativeName;
};

class LanguageUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<Language> languages MEMBER sLanguages CONSTANT FINAL)
    QML_ELEMENT

public:
    explicit LanguageUtils(QObject* parent = nullptr);

private:
    static void initLanguages();

    static QList<Language> sLanguages;
};

}
