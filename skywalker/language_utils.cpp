// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "language_utils.h"
#include "skywalker.h"
#include <QInputMethod>
#include <QLocale>
#include <QGuiApplication>
#include <unordered_set>

namespace Skywalker {

static constexpr char const* DEFAULT_LANGUAGE = "en";

Language::Language(const QString& code, const QString& nativeName) :
    mShortCode(LanguageUtils::languageCodeToShortCode(code)),
    mNativeName(nativeName)
{
}

QList<Language> LanguageUtils::sLanguages;

QString LanguageUtils::languageCodeToShortCode(const QString& languageCode)
{
    return languageCode.split('_').front();
}

LanguageUtils::LanguageUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
    initLanguages();
}

QString LanguageUtils::getDefaultPostLanguage() const
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    const auto* settings = mSkywalker->getUserSettings();
    QString lang = settings->getDefaultPostLanguage(did);

    if (!lang.isEmpty())
        return lang;

    if (!qGuiApp)
        return DEFAULT_LANGUAGE;

    auto* inputMethod = qGuiApp->inputMethod();
    lang = languageCodeToShortCode(inputMethod->locale().name());

    if (lang.isEmpty())
        return DEFAULT_LANGUAGE;

    return lang;
}

void LanguageUtils::setDefaultPostLanguage(const QString& language)
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    const QString oldLang = settings->getDefaultPostLanguage(did);

    if (language == oldLang)
        return;

    settings->setDefaultPostLanguage(did, language);

    if (language.isEmpty())
        mSkywalker->showStatusMessage(tr("Default post language cleared"), QEnums::STATUS_LEVEL_INFO);
    else
        mSkywalker->showStatusMessage(tr("Default post language set to %1").arg(language), QEnums::STATUS_LEVEL_INFO);

    emit defaultPostLanguageChanged();

    if (language.isEmpty() != oldLang.isEmpty())
        emit defaultPostLanguageSetChanged();
}

bool LanguageUtils::isDefaultPostLanguageSet() const
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    const auto* settings = mSkywalker->getUserSettings();
    const QString lang = settings->getDefaultPostLanguage(did);
    return !lang.isEmpty();
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
