// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "language_utils.h"
#include <QLocale>
#include <unordered_set>

namespace Skywalker {

Language::Language(const QString& code, const QString& nativeName) :
    mShortCode(code.split('_').front()),
    mNativeName(nativeName)
{
}

QList<Language> LanguageUtils::sLanguages;

LanguageUtils::LanguageUtils(QObject* parent) :
    QObject(parent)
{
    initLanguages();
}

void LanguageUtils::initLanguages()
{
    if (!sLanguages.empty())
        return;

    std::unordered_set<QString> codes;
    sLanguages.reserve(QLocale::Language::LastLanguage);

    // Add English separate as Qt has many languages marked as en_US making it American.
    Language langEn("en_UK", "English");
    qDebug() << "CODE:" << langEn.getShortCode() << "NAME:" << langEn.getNativeName();
    codes.insert(langEn.getShortCode());
    sLanguages.push_back(langEn);

    for (int i = 2; i <= QLocale::Language::LastLanguage; ++i)
    {
        const QLocale locale((QLocale::Language)i);
        Language lang(locale.name(), locale.nativeLanguageName());

        if (codes.contains(lang.getShortCode()))
            continue;

        codes.insert(lang.getShortCode());

        qDebug() << i << "CODE:" << lang.getShortCode() << "NAME:" << lang.getNativeName();
        sLanguages.push_back(lang);
        std::sort(sLanguages.begin(), sLanguages.end(),
                  [](const Language& lhs, const Language& rhs){
                      return lhs.getNativeName().toLower().localeAwareCompare(rhs.getNativeName().toLower()) < 0;
                  });
    }
}

}
