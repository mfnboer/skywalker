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
static constexpr int MAX_USED_LANGUAGES = 5;

Language::Language(const QString& code, const QString& nativeName) :
    mShortCode(LanguageUtils::languageCodeToShortCode(code)),
    mNativeName(nativeName)
{
}

QList<Language> LanguageUtils::sLanguages;
std::unordered_map<QString, QString> LanguageUtils::sLanguageShortCodeToNameMap;

QString LanguageUtils::languageCodeToShortCode(const QString& languageCode)
{
    return languageCode.split('_').front();
}

QString LanguageUtils::getInputLanguage()
{
    if (!qGuiApp)
        return {};

    auto* inputMethod = qGuiApp->inputMethod();
    return languageCodeToShortCode(inputMethod->locale().name());
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

    lang = getInputLanguage();

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
    {
        mSkywalker->showStatusMessage(tr("Default post language cleared"),
                                      QEnums::STATUS_LEVEL_INFO);
    }
    else
    {
        const Language l = getLanguage(language);
        mSkywalker->showStatusMessage(tr("Default post language set to %1").arg(l.getNativeName()),
                                      QEnums::STATUS_LEVEL_INFO);
    }

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

QList<Language> LanguageUtils::getUsedPostLanguages() const
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    const auto* settings = mSkywalker->getUserSettings();
    const QString defaultLang = settings->getDefaultPostLanguage(did);
    const QStringList langs = settings->getUsedPostLanguages(did);
    const QStringList contentLangs = settings->getContentLanguages(did);
    QList<Language> usedLangs;
    std::unordered_set<QString> usedShortCodes;

    for (const QString& shortCode : langs)
    {
        usedLangs.push_back(getLanguage(shortCode));
        usedShortCodes.insert(shortCode);
    }

    const QString inputLang = getInputLanguage();

    if (!inputLang.isEmpty() && !usedShortCodes.contains(inputLang))
    {
        usedLangs.push_front(getLanguage(inputLang));
        usedShortCodes.insert(inputLang);
    }

    if (!defaultLang.isEmpty() && !usedShortCodes.contains(defaultLang))
    {
        usedLangs.push_front(getLanguage(defaultLang));
        usedShortCodes.insert(defaultLang);
    }

    for (const auto& l : contentLangs)
    {
        if (!usedShortCodes.contains(l))
        {
            usedLangs.push_back(getLanguage(l));
            usedShortCodes.insert(l);
        }
    }

    if (!usedShortCodes.contains(DEFAULT_LANGUAGE))
        usedLangs.push_back(getLanguage(DEFAULT_LANGUAGE));

    return usedLangs;
}

void LanguageUtils::addUsedPostLanguage(const QString& language)
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList langs = settings->getUsedPostLanguages(did);

    if (!langs.isEmpty() && langs.front() == language)
        return;

    langs.removeOne(language);

    while (langs.size() >= MAX_USED_LANGUAGES)
        langs.pop_back();

    langs.push_front(language);
    settings->setUsedPostLanguages(did, langs);
    emit usedPostLanguagesChanged();
}

Language LanguageUtils::getLanguage(const QString& shortCode) const
{

    const QString& name = sLanguageShortCodeToNameMap.contains(shortCode) ?
        sLanguageShortCodeToNameMap.at(shortCode) : shortCode;
    return Language(shortCode, name);
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
    sLanguageShortCodeToNameMap[langEn.getShortCode()] = langEn.getNativeName();

    for (int i = 2; i <= QLocale::Language::LastLanguage; ++i)
    {
        const QLocale locale((QLocale::Language)i);
        Language lang(locale.name(), locale.nativeLanguageName());

        if (codes.contains(lang.getShortCode()))
            continue;

        codes.insert(lang.getShortCode());

        qDebug() << i << "CODE:" << lang.getShortCode() << "NAME:" << lang.getNativeName();
        sLanguages.push_back(lang);
        sLanguageShortCodeToNameMap[lang.getShortCode()] = lang.getNativeName();
    }

    std::sort(sLanguages.begin(), sLanguages.end(),
              [](const Language& lhs, const Language& rhs){
                  return lhs.getNativeName().toLower().localeAwareCompare(rhs.getNativeName().toLower()) < 0;
              });
}

}
