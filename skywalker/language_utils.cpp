// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "language_utils.h"
#include "jni_callback.h"
#include "skywalker.h"
#include <QInputMethod>
#include <QLocale>
#include <QGuiApplication>
#include <unordered_set>

namespace Skywalker {

static constexpr char const* DEFAULT_LANGUAGE = "en";
static constexpr int MAX_USED_LANGUAGES = 5;
static constexpr int MIN_LANGUAGE_IDENTIFICATION_LENGTH = 20;

int LanguageUtils::sNextRequestId = 1;

Language::Language(const QString& code, const QString& nativeName) :
    mCode(code),
    mShortCode(LanguageUtils::languageCodeToShortCode(code)),
    mNativeName(nativeName)
{
}

Language::Match Language::compare(const Language& other) const
{
    if (mCode == other.mCode)
        return Match::CODE;

    if (mShortCode == other.mShortCode)
        return Match::SHORT_CODE;

    return Match::NONE;
}

LanguageList LanguageUtils::sLanguages;
std::unordered_map<QString, QString> LanguageUtils::sLanguageShortCodeToNameMap;

QString LanguageUtils::languageCodeToShortCode(const QString& languageCode)
{
    return languageCode.split('_').front().split('-').front();
}

bool LanguageUtils::existsShortCode(const QString& shortCode)
{
    initLanguages();
    return sLanguageShortCodeToNameMap.contains(shortCode);
}

QString LanguageUtils::getLanguageName(const QString& languageCode)
{
    initLanguages();
    const auto shortCode = languageCodeToShortCode(languageCode);

    if (sLanguageShortCodeToNameMap.contains(shortCode))
        return sLanguageShortCodeToNameMap[shortCode];

    return languageCode;
}

template<typename List>
static LanguageList getLanguageList(const List& langCodes)
{
    LanguageList languages;
    languages.reserve(langCodes.size());

    for (const QString& lang : langCodes)
    {
        const QString name = LanguageUtils::getLanguageName(lang);
        languages.push_back(Language(lang, name));
    }

    return languages;
}

LanguageList LanguageUtils::getLanguages(const std::vector<QString>& langCodes)
{
    return getLanguageList(langCodes);
}

LanguageList LanguageUtils::getLanguages(const QStringList& langCodes)
{
    return getLanguageList(langCodes);
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

    if (qGuiApp)
    {
        auto* inputMethod = qGuiApp->inputMethod();
        connect(inputMethod, &QInputMethod::localeChanged, this, [this]{
            qDebug() << "Input language changed:", getInputLanguage();
            emit defaultPostLanguageChanged();
        });
    }

    auto& jniCallbackListener = JNICallbackListener::getInstance();
    connect(&jniCallbackListener, &JNICallbackListener::languageIdentified, this,
        [this](QString languageCode, int requestId){
            emit languageIdentified(languageCode, requestId);
        });
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

LanguageList LanguageUtils::getUsedPostLanguages() const
{
    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    const auto* settings = mSkywalker->getUserSettings();
    const QString defaultLang = settings->getDefaultPostLanguage(did);
    const QStringList langs = settings->getUsedPostLanguages(did);
    const QStringList contentLangs = settings->getContentLanguages(did);
    LanguageList usedLangs;
    std::unordered_set<QString> usedShortCodes;

    for (const QString& shortCode : langs)
    {
        if (shortCode.isEmpty())
            continue;

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
    if (language.isEmpty())
    {
        qDebug() << "No language";
        return;
    }

    Q_ASSERT(mSkywalker);
    const QString& did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList langs = settings->getUsedPostLanguages(did);

    if (!langs.isEmpty() && langs.front() == language)
        return;

    langs.removeOne(language);
    langs.removeAll(""); // somehow empty languages got in the list

    while (langs.size() >= MAX_USED_LANGUAGES)
        langs.pop_back();

    langs.push_front(language);
    settings->setUsedPostLanguages(did, langs);
    emit usedPostLanguagesChanged();
}

bool LanguageUtils::getDefaultLanguageNoticeSeen() const
{
    Q_ASSERT(mSkywalker);
    const auto* settings = mSkywalker->getUserSettings();
    return settings->getDefaultLanguageNoticeSeen();
}

void LanguageUtils::setDefaultLanguageNoticeSeen(bool seen)
{
    Q_ASSERT(mSkywalker);
    auto* settings = mSkywalker->getUserSettings();
    settings->setDefautlLanguageNoticeSeen(seen);
}


int LanguageUtils::identifyLanguage(QString text)
{
    if (text.length() < MIN_LANGUAGE_IDENTIFICATION_LENGTH)
        return -1;

#if defined(Q_OS_ANDROID)
    auto jsText  = QJniObject::fromString(text);
    const int requestId = sNextRequestId++;
    const QStringList excludeLanguages = mSkywalker->getUserSettings()->getExcludeDetectLanguages(mSkywalker->getUserDid());
    auto jsExcludeLanguages = QJniObject::fromString(excludeLanguages.join(','));

    // Async call to guarantee that the caller gets requestId before results from detection.
    QTimer::singleShot(0, this, [jsText, jsExcludeLanguages, requestId]{
        QJniObject::callStaticMethod<void>(
            "com/gmail/mfnboer/LanguageDetection",
            "detectLanguage",
            "(Ljava/lang/String;Ljava/lang/String;I)V",
            jsText,
            jsExcludeLanguages,
            (jint)requestId);
    });

    return requestId;
#else
    qDebug() << "Language identification not supported:" << text;
    return -1;
#endif
}

Language LanguageUtils::getLanguage(const QString& shortCode) const
{

    const QString& name = getLanguageName(shortCode);
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
        sLanguages.push_back(lang);
        sLanguageShortCodeToNameMap[lang.getShortCode()] = lang.getNativeName();
    }

    std::sort(sLanguages.begin(), sLanguages.end(),
              [](const Language& lhs, const Language& rhs){
                  return lhs.getNativeName().toLower().localeAwareCompare(rhs.getNativeName().toLower()) < 0;
              });
}

}
