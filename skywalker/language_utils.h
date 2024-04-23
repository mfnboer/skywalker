// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "wrapped_skywalker.h"
#include <QtQmlIntegration>
#include <QObject>
#include <unordered_map>

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

using LanguageList = QList<Language>;

class LanguageUtils : public WrappedSkywalker
{
    Q_OBJECT
    Q_PROPERTY(LanguageList languages MEMBER sLanguages CONSTANT FINAL)
    Q_PROPERTY(LanguageList usedLanguages READ getUsedPostLanguages NOTIFY usedPostLanguagesChanged FINAL)
    Q_PROPERTY(QString defaultPostLanguage READ getDefaultPostLanguage WRITE setDefaultPostLanguage NOTIFY defaultPostLanguageChanged FINAL)
    Q_PROPERTY(bool isDefaultPostLanguageSet READ isDefaultPostLanguageSet NOTIFY defaultPostLanguageSetChanged FINAL)
    QML_ELEMENT

public:
    static QString languageCodeToShortCode(const QString& languageCode);
    static QString getLanguageName(const QString& languageCode);
    static LanguageList getLanguages(const std::vector<QString>& langCodes);
    static QString getInputLanguage();

    explicit LanguageUtils(QObject* parent = nullptr);

    QString getDefaultPostLanguage() const;
    void setDefaultPostLanguage(const QString& language);
    bool isDefaultPostLanguageSet() const;
    LanguageList getUsedPostLanguages() const;
    Q_INVOKABLE void addUsedPostLanguage(const QString& language);
    Q_INVOKABLE bool getDefaultLanguageNoticeSeen() const;
    Q_INVOKABLE void setDefaultLanguageNoticeSeen(bool seen);

signals:
    void defaultPostLanguageChanged();
    void defaultPostLanguageSetChanged();
    void usedPostLanguagesChanged();

private:
    static void initLanguages();
    Language getLanguage(const QString& shortCode) const;

    static LanguageList sLanguages;
    static std::unordered_map<QString, QString> sLanguageShortCodeToNameMap;
};

}
