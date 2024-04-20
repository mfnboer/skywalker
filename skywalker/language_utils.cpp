// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "language_utils.h"
#include <QLocale>
#include <unordered_set>

namespace Skywalker {

Language::Language(QObject* parent) :
    QObject(parent)
{
}

Language::Language(const QString& code, const QString& nativeName, QObject* parent) :
    QObject(parent),
    mCode(code),
    mShortCode(code.split('_').front()),
    mNativeName(nativeName)
{
    mCode.replace('_', '-');
}

LanguageUtils::LanguageUtils(QObject* parent) :
    QObject(parent)
{
    initLanguages();
}

void LanguageUtils::initLanguages()
{
    std::unordered_set<QString> codes;
    mLanguages.reserve(QLocale::Language::LastLanguage);

    // Add English separate as Qt has many languages marked as en_US making it American.
    Language* langEn = new Language("en_UK", "English", this);
    qDebug() << "CODE:" << langEn->getShortCode() << "NAME:" << langEn->getNativeName();
    codes.insert(langEn->getShortCode());
    mLanguages.push_back(langEn);

    for (int i = 2; i <= QLocale::Language::LastLanguage; ++i)
    {
        const QLocale locale((QLocale::Language)i);
        Language* lang = new Language(locale.name(), locale.nativeLanguageName(), this);

        if (codes.contains(lang->getShortCode()))
        {
            delete lang;
            continue;
        }

        codes.insert(lang->getShortCode());

        qDebug() << i << "CODE:" << lang->getShortCode() << "NAME:" << lang->getNativeName();
        mLanguages.push_back(lang);
        std::sort(mLanguages.begin(), mLanguages.end(),
                  [](const Language* lhs, const Language* rhs){ return lhs->getNativeName().localeAwareCompare(rhs->getNativeName()) < 0; });
    }
}

}
