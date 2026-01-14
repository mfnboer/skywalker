// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "user_settings.h"
#include "activity_status.h"
#include "definitions.h"
#include "file_utils.h"
#include "font_downloader.h"
#include "unicode_fonts.h"
#include <atproto/lib/at_uri.h>
#include <atproto/lib/client.h>

// NOTE: do not store user defined types (Q_DECLARE_METATYPE) in settings.
// This will break OffLineMessageChecker. This runs in an Android background process.
// For some reason these types are not available in such a process.

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr char const* KEY_ALIAS_PASSWORD = "SkywalkerPass";
static constexpr double DEFAULT_FONT_SCALE = 1.0;

static constexpr char const* VALUE_TYPE_NULL = "Null";
static constexpr char const* VALUE_TYPE_DATE = "Date";
static constexpr char const* VALUE_TYPE_DATE_TIME = "DateTime";
static constexpr char const* VALUE_TYPE_BOOL = "Bool";
static constexpr char const* VALUE_TYPE_INT = "Int";
static constexpr char const* VALUE_TYPE_DOUBLE = "Double";
static constexpr char const* VALUE_TYPE_STRING = "String";
static constexpr char const* VALUE_TYPE_STRING_LIST = "StringList";
static constexpr char const* VALUE_TYPE_JSON_DOCUMENT = "JsonDocument";
static constexpr char const* VALUE_TYPE_JSON_ARRAY = "JsonArray";
static constexpr char const* VALUE_TYPE_JSON_OBJECT = "JsonObject";

QEnums::DisplayMode UserSettings::sActiveDisplayMode(QEnums::DISPLAY_MODE_LIGHT);
QString UserSettings::sDefaultBackgroundColor("white");
QString UserSettings::sDefaultTextColor("black");
QString UserSettings::sCurrentLinkColor("blue");

int UserSettings::getActiveOnlineIntervalMins()
{
    return ActivityStatus::ACTIVE_INTERVAL / 1min;
}

UserSettings::UserSettings(QObject* parent) :
    QObject(parent)
{
    qDebug() << "Settings:" << mSettings.fileName();
    mEncryption.init(KEY_ALIAS_PASSWORD);
    cleanup();
}

UserSettings::UserSettings(const QString& fileName, QObject* parent) :
    QObject(parent),
    mSettings(fileName, QSettings::defaultFormat())
{
    qDebug() << "Settings:" << mSettings.fileName();
    mEncryption.init(KEY_ALIAS_PASSWORD);
    cleanup();
}

bool UserSettings::isValidKeyPart(const QString& keyPart) const
{
    if (keyPart.indexOf('/') != -1)
    {
        qWarning() << "Invalid key part:" << keyPart;
        return false;
    }

    return true;
}

QString UserSettings::key(const QString& did, const QString& subkey) const
{
    return QString("%1/%2").arg(did, subkey);
}

QString UserSettings::key(const QString& did, const QString& subkey1, const QString& subkey2) const
{
    return QString("%1/%2/%3").arg(did, subkey1, subkey2);
}

static QString& uriToKey(QString& uri)
{
    Q_ASSERT(uri.indexOf('|') == -1);
    return uri.replace('/', '|');
}

static QString& keyToUri(QString& uri)
{
    return uri.replace('|', '/');
}

QString UserSettings::uriKey(const QString& did, const QString& subkey, QString uri) const
{
    return QString("%1/%2/%3").arg(did, subkey, uriToKey(uri));
}

QString UserSettings::displayKey(const QString& key) const
{
    const auto display = getActiveDisplayMode() == QEnums::DISPLAY_MODE_DARK ? "dark" : "light";
    return QString("%1/%2").arg(key, display);
}

QString UserSettings::labelsKey(const QString& did, const QString& labelerDid) const
{
    return QString("%1/labels/%2").arg(did, labelerDid);
}

QString UserSettings::fixedLabelerKey(const QString& did, const QString& labelerDid) const
{
    return QString("%1/fixedLabeler/%2").arg(did, labelerDid);
}

QString UserSettings::labelPolicyKey(const QString& did, QString listUri, const QString& labelerDid,
                     const QString& labelId) const
{
    const QString labeler = labelerDid.isEmpty() ? "global" : labelerDid;
    return QString("%1/labelpolicy/%2/%3/%4").arg(did, uriToKey(listUri), labeler, labelId);
}

void UserSettings::reset()
{
    mSyncFeeds.reset();
    mBlocksWithExpiry = nullptr;
    mMutesWithExpiry = nullptr;

    mHideRepliesInThreadFromUnfollowed.reset();
    mShowSelfReposts.reset();
    mShowFollowedReposts.reset();
    mShowUnknownContentLanguage.reset();
    mContentLanguages.reset();
    mShowQuotesWithBlockedPost.reset();
    mAssembleThreads.reset();

    setActiveUserDid({});
}

template<typename T>
static QJsonObject valueToJson(const QString& type, const T& value)
{
    QJsonObject json;
    json.insert("$type", type);
    json.insert("value", value);
    return json;
}

QJsonObject UserSettings::toJson() const
{
    QJsonObject json;
    const QStringList keys = mSettings.allKeys();

    for (const QString& key : keys)
    {
        qDebug() << "Key:" << key;

        if (key.endsWith("/password") ||
            key.endsWith("/rememberPassword") ||
            key.endsWith("/access") ||
            key == "activeUser")
        {
            continue;
        }

        const QVariant value = mSettings.value(key);

        if (value.isNull() || !value.isValid())
            json[key] = valueToJson(VALUE_TYPE_NULL, QString(""));
        else if (value.userType() == QMetaType::QDate)
            json[key] = valueToJson(VALUE_TYPE_DATE, value.toDate().toString(Qt::ISODate));
        else if (value.userType() == QMetaType::QDateTime)
            json[key] = valueToJson(VALUE_TYPE_DATE_TIME, value.toDateTime().toString(Qt::ISODateWithMs));
        else if (value.userType() == QMetaType::Bool)
            json[key] = valueToJson(VALUE_TYPE_BOOL, value.toBool());
        else if (value.userType() == QMetaType::Int)
            json[key] = valueToJson(VALUE_TYPE_INT, value.toInt());
        else if (value.userType() == QMetaType::Double)
            json[key] = valueToJson(VALUE_TYPE_DOUBLE, value.toDouble());
        else if (value.userType() == QMetaType::QString)
            json[key] = valueToJson(VALUE_TYPE_STRING, value.toString());
        else if (value.canConvert<QStringList>())
            json[key] = valueToJson(VALUE_TYPE_STRING_LIST, QJsonArray::fromStringList(value.toStringList()));
        else if (value.canConvert<QJsonDocument>())
            json[key] = valueToJson(VALUE_TYPE_JSON_DOCUMENT, value.toJsonDocument().array());
        else if (value.canConvert<QJsonArray>())
            json[key] = valueToJson(VALUE_TYPE_JSON_ARRAY, value.toJsonArray());
        else if (value.canConvert<QJsonObject>())
            json[key] = valueToJson(VALUE_TYPE_JSON_OBJECT, value.toJsonObject());
        else
            qWarning() << "Cannot convert key:" << key << "value:" << value << "type:" << value.userType();
    }

    return json;
}

void UserSettings::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    const QStringList keys = json.keys();

    for (const QString& key : keys)
    {
        qDebug() << "Key:" << key;
        const QJsonObject value = xjson.getRequiredJsonObject(key);
        const ATProto::XJsonObject xjsonValue(value);
        const QString type = xjsonValue.getRequiredString("$type");

        if (type == VALUE_TYPE_NULL)
            mSettings.remove(key);
        else if (type == VALUE_TYPE_DATE_TIME)
            mSettings.setValue(key, xjsonValue.getRequiredDateTime("value"));
        else if (type == VALUE_TYPE_DATE)
            mSettings.setValue(key, xjsonValue.getRequiredDate("value"));
        else if (type == VALUE_TYPE_BOOL)
            mSettings.setValue(key, xjsonValue.getRequiredBool("value"));
        else if (type == VALUE_TYPE_INT)
            mSettings.setValue(key, xjsonValue.getRequiredInt("value"));
        else if (type == VALUE_TYPE_DOUBLE)
            mSettings.setValue(key, xjsonValue.getRequiredDouble("value"));
        else if (type == VALUE_TYPE_STRING)
            mSettings.setValue(key, xjsonValue.getRequiredString("value"));
        else if (type == VALUE_TYPE_STRING_LIST)
        {
            const auto strings = xjsonValue.getRequiredStringVector("value");
            const QStringList stringList(strings.begin(), strings.end());
            mSettings.setValue(key, stringList);
        }
        else if (type == VALUE_TYPE_JSON_DOCUMENT)
        {
            const auto array = xjsonValue.getRequiredArray("value");
            const QJsonDocument jsonDoc(array);
            mSettings.setValue(key, jsonDoc);
        }
        else if (type == VALUE_TYPE_JSON_ARRAY)
            mSettings.setValue(key, xjsonValue.getRequiredArray("value"));
        else if (type == VALUE_TYPE_JSON_OBJECT)
            mSettings.setValue(key, xjsonValue.getRequiredJsonObject("value"));
        else
            qWarning() << "Unknown type:" << type << "key:" << key << "value:" << json[key];
    }
}

QString UserSettings::save(const QUrl& fileUri) const
{
    auto file = FileUtils::openFile(fileUri, FileUtils::FileMode::WRITE_ONLY);

    if (!file)
    {
        qWarning() << "Cannot create file:" << fileUri;
        return tr("Cannot create settings file");
    }

    const auto json = QJsonDocument(toJson());
    const QByteArray data = json.toJson(QJsonDocument::Compact);

    if (file->write(data) == -1)
    {
        qWarning() << "Failed to write:" << fileUri;
        return tr("Failed to save settings");
    }

    file->close();
    qDebug() << "Saved settings to:" << fileUri;
    return {};
}

QString UserSettings::load(const QUrl& fileUri)
{
    auto file = FileUtils::openFile(fileUri, FileUtils::FileMode::READ_ONLY);

    if (!file)
    {
        qWarning() << "Cannot open file:" << fileUri;
        return tr("Cannot open settings file");
    }

    const QByteArray data = file->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    if (jsonDoc.isNull())
    {
        qWarning() << "Not valid JSON:" << fileUri;
        return tr("Cannot read settings file");
    }

    QJsonObject json = jsonDoc.object();

    if (json.isEmpty())
    {
        qWarning() << "JSON object is missing:" << fileUri;
        return tr("Not a valid settings file: JSON object missing");
    }

    try {
        fromJson(json);
    } catch (ATProto::InvalidJsonException& e) {
        qWarning() << "Invalid JSON:" << e.msg();
        return tr("Not a valid settings file: %1").arg(e.msg());
    }

    qDebug() << "Loaded settings from:" << fileUri;

    emit contentLanguageFilterChanged();
    emit backgroundColorChanged();
    emit textColorChanged();
    emit accentColorChanged();
    emit linkColorChanged();
    emit threadStyleChanged();
    emit threadColorChanged();
    emit fontScaleChanged();
    emit favoritesBarPositionChanged();
    emit giantEmojisChanged();
    emit songlinkEnabledChanged();
    emit wrapLabelsChanged();
    emit showFollowsStatusChanged();
    emit showFollowsActiveStatusChanged();
    emit showFeedbackButtonsChanged();
    emit sideBarTypeChanged();
    emit gifAutoPlayChanged();
    emit videoSoundChanged();
    emit videoAutoPlayChanged();
    emit videoAutoLoadChanged();
    emit videoLoopPlayChanged();
    emit videoQualityChanged();
    emit videoStreamingEnabledChanged();
    emit scriptRecognitionChanged();
    emit showTrendingTopicsChanged();
    emit showSuggestedFeedsChanged();
    emit showSuggestedUsersChanged();
    emit showSuggestedStarterPacksChanged();
    emit blocksWithExpiryChanged();
    emit mutesWithExpiryChanged();
    emit notificationsForAllAccountsChanged();

    return {};
}

void UserSettings::setActiveDisplayMode(QEnums::DisplayMode mode)
{
    sActiveDisplayMode = mode;
    emit threadColorChanged();
    emit backgroundColorChanged();
    emit textColorChanged();
    emit accentColorChanged();
    emit linkColorChanged();
}

void UserSettings::setDefaultBackgroundColor(const QString& color)
{
    qDebug() << "Default background color:" << color;
    sDefaultBackgroundColor = color;
}

QList<BasicProfile> UserSettings::getUserList() const
{
    const auto didList = getUserDidList();
    QList<BasicProfile> userList;

    for (const auto& did : didList)
    {
        BasicProfile profile(
            did,
            getHandle(did),
            getDisplayName(did),
            getAvatar(did));

        userList.append(profile);
    }

    return userList;
}

QList<BasicProfile> UserSettings::getUserListWithAddAccount() const
{
    auto userList = getUserList();
    const BasicProfile addAccount("", "", tr("Add other acount"), "");
    userList.append(addAccount);
    return userList;
}

QStringList UserSettings::getUserDidList() const
{
    return mSettings.value("users").toStringList();
}

BasicProfile UserSettings::getUser(const QString& did) const
{
    BasicProfile profile(
        did,
        getHandle(did),
        getDisplayName(did),
        getAvatar(did));

    return profile;
}

void UserSettings::setActiveUserDid(const QString& did)
{
    mSettings.setValue("activeUser", did);
}

QString UserSettings::getActiveUserDid() const
{
    return mSettings.value("activeUser").toString();
}

void UserSettings::addUser(const QString& did, const QString& host)
{
    auto users = getUserDidList();

    if (users.contains(did))
    {
        qDebug() << "User already added:" << did;
        return;
    }

    users.append(did);
    users.sort();
    mSettings.setValue("users", users);
    mSettings.setValue(key(did, "host"), host);
}

void UserSettings::removeUser(const QString& did)
{
    auto users = getUserDidList();
    users.removeOne(did);
    mSettings.setValue("users", users);
    clearCredentials(did);

    const auto activeUser = getActiveUserDid();
    if (did == activeUser)
        setActiveUserDid({});
}

QString UserSettings::getHost(const QString& did) const
{
    return mSettings.value(key(did, "host")).toString();
}

void UserSettings::setRememberPassword(const QString& did, bool enable)
{
    mSettings.setValue(key(did, "rememberPassword"), enable);

    if (!enable)
        mSettings.remove(key(did, "password"));

    sync();
}

bool UserSettings::getRememberPassword(const QString& did) const
{
    return mSettings.value(key(did, "rememberPassword"), false).toBool();
}

void UserSettings::savePassword(const QString& did, const QString& password)
{
    if (!getRememberPassword(did))
    {
        qWarning() << "Password saving is not enabled";
        return;
    }

    const QByteArray encryptedPassword = mEncryption.encrypt(password, KEY_ALIAS_PASSWORD);
    mSettings.setValue(key(did, "password"), encryptedPassword);
}

QString UserSettings::getPassword(const QString& did) const
{
    if (!getRememberPassword(did))
    {
        qWarning() << "Password saving is not enabled";
        return {};
    }

    const QByteArray encryptedPassword = mSettings.value(key(did, "password")).toByteArray();

    if (encryptedPassword.isEmpty())
        return {};

    return mEncryption.decrypt(encryptedPassword, KEY_ALIAS_PASSWORD);
}

QString UserSettings::getHandle(const QString& did) const
{
    return mSettings.value(key(did, "handle")).toString();
}

void UserSettings::saveDisplayName(const QString& did, const QString& displayName)
{
    mSettings.setValue(key(did, "displayName"), displayName);
}

QString UserSettings::getDisplayName(const QString& did) const
{
    return mSettings.value(key(did, "displayName")).toString();
}

void UserSettings::saveAvatar(const QString& did, const QString& avatar)
{
    mSettings.setValue(key(did, "avatar"), avatar);
}

QString UserSettings::getAvatar(const QString& did) const
{
    return mSettings.value(key(did, "avatar")).toString();
}

void UserSettings::setServiceAppView(const QString& did, const QString& service)
{
    if (service != getServiceAppView(did))
    {
        if (service == ATProto::Client::SERVICE_APP_VIEW)
            mSettings.remove(key(did, "serviceAppView"));
        else
            mSettings.setValue(key(did, "serviceAppView"), service);

        emit serviceAppViewChanged(did);
    }
}

QString UserSettings::getServiceAppView(const QString& did) const
{
    return mSettings.value(key(did, "serviceAppView"), ATProto::Client::SERVICE_APP_VIEW).toString();
}

QString UserSettings::getDefaultServiceAppView() const
{
    return ATProto::Client::SERVICE_APP_VIEW;
}

void UserSettings::setServiceChat(const QString& did, const QString& service)
{
    if (service != getServiceChat(did))
    {
        if (service == ATProto::Client::SERVICE_CHAT)
            mSettings.remove(key(did, "serviceChat"));
        else
            mSettings.setValue(key(did, "serviceChat"), service);

        emit serviceChatChanged(did);
    }
}

QString UserSettings::getServiceChat(const QString& did) const
{
    return mSettings.value(key(did, "serviceChat"), ATProto::Client::SERVICE_CHAT).toString();
}

QString UserSettings::getDefaultServiceChat() const
{
    return ATProto::Client::SERVICE_CHAT;
}

void UserSettings::setServiceVideoHost(const QString& did, const QString& host)
{
    if (host != getServiceVideoHost(did))
    {
        if (host == ATProto::Client::SERVICE_VIDEO_HOST)
            mSettings.remove(key(did, "serviceVideoHost"));
        else
            mSettings.setValue(key(did, "serviceVideoHost"), host);

        emit serviceVideoHostChanged(did);
    }
}

QString UserSettings::getServiceVideoHost(const QString& did) const
{
    return mSettings.value(key(did, "serviceVideoHost"), ATProto::Client::SERVICE_VIDEO_HOST).toString();
}

QString UserSettings::getDefaultServiceVideoHost() const
{
    return ATProto::Client::SERVICE_VIDEO_HOST;
}

void UserSettings::setServiceVideoDid(const QString& did, const QString& serviceDid)
{
    if (serviceDid != getServiceVideoDid(did))
    {
        if (serviceDid == ATProto::Client::SERVICE_VIDEO_DID)
            mSettings.remove(key(did, "serviceVideoDid"));
        else
            mSettings.setValue(key(did, "serviceVideoDid"), serviceDid);

        emit serviceVideoDidChanged(did);
    }
}

QString UserSettings::getServiceVideoDid(const QString& did) const
{
    return mSettings.value(key(did, "serviceVideoDid"), ATProto::Client::SERVICE_VIDEO_DID).toString();
}

QString UserSettings::getDefaultServiceVideoDid() const
{
    return ATProto::Client::SERVICE_VIDEO_DID;
}

void UserSettings::saveSession(const ATProto::ComATProtoServer::Session& session)
{
    qDebug() << "Save session:" << session.mHandle;
    mSettings.setValue(key(session.mDid, "handle"), session.mHandle);
    mSettings.setValue(key(session.mDid, "access"), session.mAccessJwt);
    mSettings.setValue(key(session.mDid, "refresh"), session.mRefreshJwt);
    mSettings.setValue(key(session.mDid, "2FA"), session.mEmailAuthFactor);
    qDebug() << "Session saved:" << session.mHandle;
}

ATProto::ComATProtoServer::Session UserSettings::getSession(const QString& did) const
{
    ATProto::ComATProtoServer::Session session;
    session.mDid = did;
    session.mHandle = mSettings.value(key(did, "handle")).toString();
    session.mAccessJwt = mSettings.value(key(did, "access")).toString();
    session.mRefreshJwt = mSettings.value(key(did, "refresh")).toString();
    session.mEmailAuthFactor = mSettings.value(key(did, "2FA"), false).toBool();
    return session;
}

void UserSettings::clearTokens(const QString& did)
{
    qDebug() << "Clear tokens:" << did;
    mSettings.remove(key(did, "access"));
    mSettings.remove(key(did, "refresh"));
    mSettings.sync();
}

void UserSettings::clearCredentials(const QString& did)
{
    qDebug() << "Clear credentials:" << did;
    setRememberPassword(did, false);
    mSettings.remove(key(did, "password"));
    clearTokens(did);
}

void UserSettings::saveSyncTimestamp(const QString& did, QDateTime timestamp)
{
    mSettings.setValue(key(did, "syncTimestamp"), timestamp);
}

QDateTime UserSettings::getSyncTimestamp(const QString& did) const
{
    return mSettings.value(key(did, "syncTimestamp")).toDateTime();
}

void UserSettings::saveSyncCid(const QString& did, const QString& cid)
{
    mSettings.setValue(key(did, "syncCid"), cid);
}

QString UserSettings::getSyncCid(const QString& did) const
{
    return mSettings.value(key(did, "syncCid")).toString();
}

void UserSettings::saveSyncOffsetY(const QString& did, int offsetY)
{
    mSettings.setValue(key(did, "syncOffsetY"), offsetY);
}

int UserSettings::getSyncOffsetY(const QString& did) const
{
    return mSettings.value(key(did, "syncOffsetY"), 0).toInt();
}

void UserSettings::setTimelineViews(const QString& did, const QJsonObject postFilters)
{
    mSettings.setValue(key(did, "timelineViews"), postFilters);
}

QJsonObject UserSettings::getTimelineViews(const QString& did) const
{
    return mSettings.value(key(did, "timelineViews")).toJsonObject();
}

void UserSettings::saveFeedSyncTimestamp(const QString& did, const QString& feedUri, QDateTime timestamp)
{
    mSettings.setValue(uriKey(did, "syncFeedTimestamp", feedUri), timestamp);
}

QDateTime UserSettings::getFeedSyncTimestamp(const QString& did, const QString& feedUri) const
{
    return mSettings.value(uriKey(did, "syncFeedTimestamp", feedUri)).toDateTime();
}

void UserSettings::saveFeedSyncCid(const QString& did, const QString& feedUri, const QString& cid)
{
    mSettings.setValue(uriKey(did, "syncFeedCid", feedUri), cid);
}

QString UserSettings::getFeedSyncCid(const QString& did, const QString& feedUri) const
{
    return mSettings.value(uriKey(did, "syncFeedCid", feedUri)).toString();
}

void UserSettings::saveFeedSyncOffsetY(const QString& did, const QString& feedUri, int offsetY)
{
    mSettings.setValue(uriKey(did, "syncFeedOffsetY", feedUri), offsetY);
}

int UserSettings::getFeedSyncOffsetY(const QString& did, const QString& feedUri) const
{
    return mSettings.value(uriKey(did, "syncFeedOffsetY", feedUri), 0).toInt();
}

void UserSettings::addSyncFeed(const QString& did, const QString& feedUri)
{
    getSyncFeeds(did);
    mSyncFeeds->insert(feedUri);
    const QStringList uris(mSyncFeeds->begin(), mSyncFeeds->end());
    mSettings.setValue(key(did, "syncFeeds"), uris);
}

void UserSettings::removeSyncFeed(const QString& did, const QString& feedUri)
{
    getSyncFeeds(did);
    mSyncFeeds->erase(feedUri);
    const QStringList uris(mSyncFeeds->begin(), mSyncFeeds->end());
    mSettings.setValue(key(did, "syncFeeds"), uris);

    mSettings.remove(uriKey(did, "syncFeedTimestamp", feedUri));
    mSettings.remove(uriKey(did, "syncFeedCid", feedUri));
    mSettings.remove(uriKey(did, "syncFeedOffsetY", feedUri));
}

const std::unordered_set<QString>& UserSettings::getSyncFeeds(const QString& did) const
{
    if (mSyncFeeds)
        return *mSyncFeeds;

    QStringList feeds = mSettings.value(key(did, "syncFeeds")).toStringList();
    const_cast<UserSettings*>(this)->mSyncFeeds = std::unordered_set<QString>(feeds.begin(), feeds.end());
    return *mSyncFeeds;
}

bool UserSettings::mustSyncFeed(const QString& did, const QString& feedUri) const
{
    const auto& feedUris = getSyncFeeds(did);
    return feedUris.contains(feedUri);
}

void UserSettings::setFeedViewMode(const QString& did, const QString& feedUri, QEnums::ContentMode mode)
{
    if (mode != QEnums::CONTENT_MODE_UNSPECIFIED)
        mSettings.setValue(uriKey(did, "feedViewMode", feedUri), (int)mode);
    else
        mSettings.remove(uriKey(did, "feedViewMode", feedUri));
}

static QEnums::ContentMode intToContentMode(int mode)
{
    if (mode < QEnums::CONTENT_MODE_UNSPECIFIED || mode > QEnums::CONTENT_MODE_LAST)
        return QEnums::CONTENT_MODE_UNSPECIFIED;

    if (mode >= QEnums::CONTENT_MODE_UNKNOWN && mode < QEnums::CONTENT_MODE_MEDIA)
        return QEnums::CONTENT_MODE_UNSPECIFIED;

    return QEnums::ContentMode(mode);
}

QEnums::ContentMode UserSettings::getFeedViewMode(const QString& did, const QString& feedUri)
{
    const int mode = mSettings.value(uriKey(did, "feedViewMode", feedUri), (int)QEnums::CONTENT_MODE_UNSPECIFIED).toInt();
    return intToContentMode(mode);
}

QStringList UserSettings::getFeedViewUris(const QString& did, const QString& feedKey) const
{
    const_cast<QSettings&>(mSettings).beginGroup(key(did, feedKey));
    QStringList uris = mSettings.allKeys();
    const_cast<QSettings&>(mSettings).endGroup();

    for (auto& uri : uris)
        keyToUri(uri);

    return uris;
}

QStringList UserSettings::getFeedViewModeUris(const QString& did) const
{
    return getFeedViewUris(did, "feedViewMode");
}

void UserSettings::setSearchFeedViewMode(const QString& did, const QString& searchQuery, QEnums::ContentMode mode)
{
    if (!isValidKeyPart(searchQuery))
        return;

    if (mode != QEnums::CONTENT_MODE_UNSPECIFIED)
        mSettings.setValue(key(did, "searchFeedViewMode", searchQuery), (int)mode);
    else
        mSettings.remove(key(did, "searchFeedViewMode", searchQuery));
}

QEnums::ContentMode UserSettings::getSearchFeedViewMode(const QString& did, const QString& searchQuery)
{
    if (!isValidKeyPart(searchQuery))
        return QEnums::CONTENT_MODE_UNSPECIFIED;

    const int mode = mSettings.value(key(did, "searchFeedViewMode", searchQuery), (int)QEnums::CONTENT_MODE_UNSPECIFIED).toInt();
    return intToContentMode(mode);
}

QStringList UserSettings::getSearchFeedViewSearchQueries(const QString& did, const QString& feedKey) const
{
    const_cast<QSettings&>(mSettings).beginGroup(key(did, feedKey));
    QStringList searchQueries = mSettings.allKeys();
    const_cast<QSettings&>(mSettings).endGroup();

    return searchQueries;
}

QStringList UserSettings::getSearchFeedViewModeSearchQueries(const QString& did) const
{
    return getSearchFeedViewSearchQueries(did, "searchFeedViewMode");
}

Q_INVOKABLE void UserSettings::setFeedHideReplies(const QString& did, const QString& feedUri, bool hide)
{
    if (getFeedHideReplies(did, feedUri) == hide)
        return;

    if (hide)
        mSettings.setValue(uriKey(did, "feedhideReplies", feedUri), hide);
    else
        mSettings.remove(uriKey(did, "feedhideReplies", feedUri));

    emit feedHideRepliesChanged(did, feedUri);
}

Q_INVOKABLE bool UserSettings::getFeedHideReplies(const QString& did, const QString& feedUri) const
{
        return mSettings.value(uriKey(did, "feedhideReplies", feedUri), false).toBool();
}

QStringList UserSettings::getFeedHideRepliesUris(const QString& did) const
{
    return getFeedViewUris(did, "feedhideReplies");
}

Q_INVOKABLE void UserSettings::setFeedHideFollowing(const QString& did, const QString& feedUri, bool hide)
{
    if (getFeedHideFollowing(did, feedUri) == hide)
        return;

    if (hide)
        mSettings.setValue(uriKey(did, "feedhideFollowing", feedUri), hide);
    else
        mSettings.remove(uriKey(did, "feedhideFollowing", feedUri));

    emit feedHideFollowingChanged(did, feedUri);
}

Q_INVOKABLE bool UserSettings::getFeedHideFollowing(const QString& did, const QString& feedUri) const
{
        return mSettings.value(uriKey(did, "feedhideFollowing", feedUri), false).toBool();
}

QStringList UserSettings::getFeedHideFollowingUris(const QString& did) const
{
    return getFeedViewUris(did, "feedhideFollowing");
}

void UserSettings::setUserOrderedPinnedFeeds(const QString& did, QStringList favoriteKeys)
{
    mSettings.setValue(key(did, "userOrderedPinnedFeeds"), favoriteKeys);
}

QStringList UserSettings::getUserOrderedPinnedFeed(const QString& did) const
{
    return mSettings.value(key(did, "userOrderedPinnedFeeds")).toStringList();
}

void UserSettings::updateLastSignInTimestamp(const QString& did)
{
    mSettings.setValue(key(did, "lastSignInTimestamp"), QDateTime::currentDateTime());
}

QDateTime UserSettings::getLastSignInTimestamp(const QString& did) const
{
    return mSettings.value(key(did, "lastSignInTimestamp")).toDateTime();
}

void UserSettings::setLastViewedFeed(const QString& did, const QString& uri)
{
    mSettings.setValue(key(did, "lastViewedFeed"), uri);
}

QString UserSettings::getLastViewedFeed(const QString& did) const
{
    return mSettings.value(key(did, "lastViewedFeed"), HOME_FEED).toString();
}

void UserSettings::setHideLists(const QString& did, const QStringList& listUris)
{
    mSettings.setValue(key(did, "hideLists"), listUris);
}

QStringList UserSettings::getHideLists(const QString& did) const
{
    return mSettings.value(key(did, "hideLists")).toStringList();
}

void UserSettings::setContentLabelPref(
    const QString& did, const QString& listUri, const QString& labelerDid,
    const QString& labelId, QEnums::ContentPrefVisibility pref)
{
    mSettings.setValue(labelPolicyKey(did, listUri, labelerDid, labelId), (int)pref);
}

QEnums::ContentPrefVisibility UserSettings::getContentLabelPref(
    const QString& did, const QString& listUri,
    const QString& labelerDid, const QString& labelId) const
{
    const int pref = mSettings.value(labelPolicyKey(did, listUri, labelerDid, labelId), int(QEnums::CONTENT_PREF_VISIBILITY_UNKNOWN)).toInt();

    if (pref < 0 || pref > QEnums::CONTENT_PREF_VISIBILITY_LAST)
        return QEnums::CONTENT_PREF_VISIBILITY_UNKNOWN;

    return QEnums::ContentPrefVisibility(pref);
}

void UserSettings::removeContentLabelPref(const QString& did, const QString& listUri,
                                          const QString& labelerDid, const QString& labelId)
{
    mSettings.remove(labelPolicyKey(did, listUri, labelerDid, labelId));
}

void UserSettings::removeContentLabelPrefList(const QString& did, QString listUri)
{
    qDebug() << "Remove content label prefs list:" << listUri << "did:" << did;
    mSettings.remove(key(did, "labelpolicy", uriToKey(listUri)));
}

std::vector<std::tuple<QString, QString, QString>> UserSettings::getContentLabelPrefKeys(const QString& did) const
{
    const_cast<QSettings&>(mSettings).beginGroup(key(did, "labelpolicy"));
    const QStringList keys = mSettings.allKeys();
    const_cast<QSettings&>(mSettings).endGroup();

    std::vector<std::tuple<QString, QString, QString>> result;
    result.reserve(keys.size());

    for (const auto& key : keys)
    {
        auto subKeys = key.split('/');

        Q_ASSERT(!subKeys.empty());
        if (subKeys.size() != 3)
        {
            qWarning() << "Invalid key:" << key;
            continue;
        }

        const auto& listUri = keyToUri(subKeys[0]);
        const auto labelerDid = subKeys[1] == "global" ? "" : subKeys[1];
        const auto& labelId = subKeys[2];

        result.push_back({listUri, labelerDid, labelId});
    }

    return result;
}

QStringList UserSettings::getContentLabelPrefListUris(const QString& did) const
{
    std::unordered_set<QString> uriSet;
    auto keys = getContentLabelPrefKeys(did);

    for (auto& [listUri, _, __] : keys)
        uriSet.insert(keyToUri(listUri));

    return QStringList{uriSet.begin(), uriSet.end()};
}

void UserSettings::setBookmarks(const QString& did, const QStringList& bookmarks)
{
    mSettings.setValue(key(did, "bookmarks"), bookmarks);
}

QStringList UserSettings::getBookmarks(const QString& did) const
{
    return mSettings.value(key(did, "bookmarks")).toStringList();
}

void UserSettings::setBookmarksMigrationAttempts(const QString& did, int attempts)
{
    mSettings.setValue(key(did, "bookmarksMigrationAttempts"), attempts);
}

int UserSettings::getBookmarksMigrationAttempts(const QString& did) const
{
    return mSettings.value(key(did, "bookmarksMigrationAttempts"), 0).toInt();
}

QStringList UserSettings::getMutedWords(const QString& did) const
{
    return mSettings.value(key(did, "mutedWords")).toStringList();
}

void UserSettings::removeMutedWords(const QString& did)
{
    qDebug() << "Remove locally stored muted words";
    mSettings.remove(key(did, "mutedWords"));
    mSettings.remove("mutedWordsNoticeSeen");
}

UriWithExpirySet* UserSettings::getBlocksWithExpiry()
{
    const auto did = getActiveUserDid();
    return getBlocksWithExpiry(did);
}

UriWithExpirySet* UserSettings::getBlocksWithExpiry(const QString& did)
{
    if (!mBlocksWithExpiry)
    {
        mBlocksWithExpiry.reset(new UriWithExpirySet(this));
        const auto jsonArray = mSettings.value(key(did, "blocksWithExpiry")).toJsonArray();
        mBlocksWithExpiry->fromJson(jsonArray);
    }

    return mBlocksWithExpiry.get();
}

void UserSettings::addBlockWithExpiry(const QString& did, const UriWithExpiry& block)
{
    qDebug() << "Add block:" << block.getUri() << block.getExpiry();
    auto* blocks = getBlocksWithExpiry(did);
    blocks->insert(block);
    mSettings.setValue(key(did, "blocksWithExpiry"), blocks->toJson());
    emit blocksWithExpiryChanged();
}

bool UserSettings::removeBlockWithExpiry(const QString& did, const QString& blockUri)
{
    qDebug() << "Remove block:" << blockUri;
    auto* blocks = getBlocksWithExpiry(did);
    const bool removed = blocks->remove(blockUri);
    mSettings.setValue(key(did, "blocksWithExpiry"), blocks->toJson());
    emit blocksWithExpiryChanged();
    return removed;
}

UriWithExpirySet* UserSettings::getMutesWithExpiry()
{
    const auto did = getActiveUserDid();
    return getMutesWithExpiry(did);
}

UriWithExpirySet* UserSettings::getMutesWithExpiry(const QString& did)
{
    if (!mMutesWithExpiry)
    {
        mMutesWithExpiry.reset(new UriWithExpirySet(this));
        const auto jsonArray = mSettings.value(key(did, "mutesWithExpiry")).toJsonArray();
        mMutesWithExpiry->fromJson(jsonArray);
    }

    return mMutesWithExpiry.get();
}

void UserSettings::addMuteWithExpiry(const QString& did, const UriWithExpiry& mute)
{
    qDebug() << "Add mute:" << mute.getUri() << mute.getExpiry();
    auto* mutes = getMutesWithExpiry(did);
    mutes->insert(mute);
    mSettings.setValue(key(did, "mutesWithExpiry"), mutes->toJson());
    emit mutesWithExpiryChanged();
}

bool UserSettings::removeMuteWithExpiry(const QString& did, const QString& muteDid)
{
    qDebug() << "Remove mute:" << muteDid;
    auto* mutes = getMutesWithExpiry(did);
    const bool removed = mutes->remove(muteDid);
    mSettings.setValue(key(did, "mutesWithExpiry"), mutes->toJson());
    emit mutesWithExpiryChanged();
    return removed;
}

void UserSettings::setDisplayMode(QEnums::DisplayMode displayMode)
{
    mSettings.setValue("displayMode", (int)displayMode);
}

QEnums::DisplayMode UserSettings::getDisplayMode() const
{
    const int mode = mSettings.value("displayMode", (int)QEnums::DISPLAY_MODE_SYSTEM).toInt();

    if (mode < QEnums::DISPLAY_MODE_SYSTEM || mode > QEnums::DISPLAY_MODE_DARK)
        return QEnums::DISPLAY_MODE_SYSTEM;

    return QEnums::DisplayMode(mode);
}

void UserSettings::resetBackgroundColor()
{
    mSettings.remove(displayKey("backgroundColor"));
    emit backgroundColorChanged();
}

void UserSettings::setBackgroundColor(const QString& color)
{
    if (getBackgroundColor() != color)
    {
        mSettings.setValue(displayKey("backgroundColor"), color);
        emit backgroundColorChanged();
    }
}

QString UserSettings::getBackgroundColor() const
{
    const auto color = mSettings.value(displayKey("backgroundColor"), getDefaultBackgroundColor()).toString();
    qDebug() << "Get background color:" << color;
    return color;
}

void UserSettings::resetTextColor()
{
    mSettings.remove(displayKey("textColor"));
    emit textColorChanged();
}

void UserSettings::setTextColor(const QString& color)
{
    if (getTextColor() != color)
    {
        mSettings.setValue(displayKey("textColor"), color);
        emit textColorChanged();
    }
}

QString UserSettings::getTextColor() const
{
    return mSettings.value(displayKey("textColor"), getDefaultTextColor()).toString();
}

void UserSettings::resetAccentColor()
{
    mSettings.remove(displayKey("accentColor"));
    emit accentColorChanged();
}

void UserSettings::setAccentColor(const QString& color)
{
    if (getAccentColor() != color)
    {
        mSettings.setValue(displayKey("accentColor"), color);
        emit accentColorChanged();
    }
}

QString UserSettings::getAccentColor() const
{
    const QString defaultColor = getActiveDisplayMode() == QEnums::DISPLAY_MODE_DARK ? "#58a6ff" : "blue";
    return mSettings.value(displayKey("accentColor"), defaultColor).toString();
}

void UserSettings::resetLinkColor()
{
    mSettings.remove(displayKey("linkColor"));
    setCurrentLinkColor(getLinkColor());
    emit linkColorChanged();
}

void UserSettings::setLinkColor(const QString& color)
{
    if (getLinkColor() != color)
    {
        mSettings.setValue(displayKey("linkColor"), color);
        setCurrentLinkColor(color);
        emit linkColorChanged();
    }
}

QString UserSettings::getLinkColor() const
{
    const QString defaultColor = getActiveDisplayMode() == QEnums::DISPLAY_MODE_DARK ? "#58a6ff" : "blue";
    return mSettings.value(displayKey("linkColor"), defaultColor).toString();
}

void UserSettings::setThreadStyle(QEnums::ThreadStyle threadStyle)
{
    const auto oldStyle = getThreadStyle();
    mSettings.setValue("threadStyle", (int)threadStyle);

    if (oldStyle != threadStyle)
        emit threadStyleChanged();
}

QEnums::ThreadStyle UserSettings::getThreadStyle() const
{
    const int style = mSettings.value("threadStyle", (int)QEnums::THREAD_STYLE_LINE).toInt();

    if (style < QEnums::THREAD_STYLE_BAR || style > QEnums::THREAD_STYLE_LINE)
        return QEnums::THREAD_STYLE_LINE;

    return QEnums::ThreadStyle(style);
}

void UserSettings::resetThreadColor()
{
    if (mSettings.contains("threadColor"))
    {
        mSettings.remove(displayKey("threadColor"));
        emit threadColorChanged();
    }
}

void UserSettings::setThreadColor(const QString& color)
{
    if (getThreadColor() != color)
    {
        mSettings.setValue(displayKey("threadColor"), color);
        emit threadColorChanged();
    }
}

QString UserSettings::getThreadColor() const
{
    const QString defaultColor = getActiveDisplayMode() == QEnums::DISPLAY_MODE_DARK ? "#000080" : "#8080ff";
    return mSettings.value(displayKey("threadColor"), defaultColor).toString();
}

void UserSettings::setFontScale(double scale)
{
    if (scale != getFontScale())
    {
        if (scale == DEFAULT_FONT_SCALE)
            mSettings.remove("fontScale");
        else
            mSettings.setValue("fontScale", scale);

        emit fontScaleChanged();
    }
}

double UserSettings::getFontScale() const
{
    const auto fontScale = mSettings.value("fontScale", DEFAULT_FONT_SCALE).toDouble();
    return fontScale > 0 ? fontScale : 1.0;
}

float UserSettings::getDeviceFontScale() const
{
    static const float deviceFontScale = FontDownloader::getFontScale();
    return deviceFontScale > 0 ? deviceFontScale : 1.0;
}

void UserSettings::setFavoritesBarPosition(QEnums::FavoritesBarPosition position)
{
    if (getFavoritesBarPosition() != position)
    {
        mSettings.setValue("favoritesBarPosition", (int)position);
        emit favoritesBarPositionChanged();
    }
}

QEnums::FavoritesBarPosition UserSettings::getFavoritesBarPosition() const
{
    const int position = mSettings.value("favoritesBarPosition", (int)QEnums::FAVORITES_BAR_POSITION_TOP).toInt();

    if (position < 0 || position > QEnums::FAVORITES_BAR_POSITION_LAST)
        return QEnums::FAVORITES_BAR_POSITION_TOP;

    return QEnums::FavoritesBarPosition(position);
}

void UserSettings::setPostButtonRelativeX(double x)
{
    mSettings.setValue("postButtonRelativeX", x);
}

double UserSettings::getPostButtonRelativeX() const
{
    return mSettings.value("postButtonRelativeX", 1.0).toDouble();
}

void UserSettings::setPortraitSideBarWidth(int width)
{
    mSettings.setValue("portraitSideBarWidth", width);
}

int UserSettings::getPortraitSideBarWidth() const
{
    return mSettings.value("portraitSideBarWidth", 200).toInt();
}

void UserSettings::setLandscapeSideBarWidth(int width)
{
    mSettings.setValue("landscapeSideBarWidth", width);
}

int UserSettings::getLandscapeSideBarWidth() const
{
    return mSettings.value("landscapeSideBarWidth", 200).toInt();
}

void UserSettings::setSideBarType(QEnums::SideBarType sideBarType)
{
    if (sideBarType == getSideBarType())
        return;

    mSettings.setValue("sideBarType", int(sideBarType));
    emit sideBarTypeChanged();
}

QEnums::SideBarType UserSettings::getSideBarType() const
{
    const int sideBarType = mSettings.value("sideBarType", int(QEnums::SIDE_BAR_LANDSCAPE)).toInt();

    if (sideBarType < 0 || sideBarType > QEnums::SIDE_BAR_LAST)
        return QEnums::SIDE_BAR_LANDSCAPE;

    return QEnums::SideBarType(sideBarType);
}

void UserSettings::setGifAutoPlay(bool autoPlay)
{
    if (autoPlay == getGifAutoPlay())
        return;

    mSettings.setValue("gifAutoPlay", autoPlay);
    emit gifAutoPlayChanged();
}

bool UserSettings::getGifAutoPlay() const
{
    return mSettings.value("gifAutoPlay", true).toBool();
}

void UserSettings::setVideoStreamingEnabled(bool enabled)
{
    if (enabled == getVideoStreamingEnabled())
        return;

    mSettings.setValue("videoStreamingEnabled", enabled);
    emit videoStreamingEnabledChanged();
}

bool UserSettings::getVideoStreamingEnabled() const
{
    return mSettings.value("videoStreamingEnabled", true).toBool();
    return false;
}

void UserSettings::setVideoQuality(QEnums::VideoQuality quality)
{
    if (quality == getVideoQuality())
        return;

    mSettings.setValue("videoQuality", (int)quality);
    emit videoQualityChanged();
}

QEnums::VideoQuality UserSettings::getVideoQuality() const
{
    int quality = mSettings.value("videoQuality", (int)QEnums::VIDEO_QUALITY_HD_WIFI).toInt();

    if (quality < 0 || quality > (int)QEnums::VIDEO_QUALITY_LAST)
        return QEnums::VIDEO_QUALITY_HD_WIFI;

    return QEnums::VideoQuality(quality);
}

void UserSettings::setVideoAutoPlay(bool autoPlay)
{
    if (autoPlay == getVideoAutoPlay())
        return;

    mSettings.setValue("videoAutoPlay", autoPlay);
    emit videoAutoPlayChanged();
}

bool UserSettings::getVideoAutoPlay() const
{
    return mSettings.value("videoAutoPlay", false).toBool();
}

void UserSettings::setVideoAutoLoad(bool autoLoad)
{
    if (autoLoad == getVideoAutoLoad())
        return;

    mSettings.setValue("videoAutoLoad", autoLoad);
    emit videoAutoLoadChanged();
}

bool UserSettings::getVideoAutoLoad() const
{
    return mSettings.value("videoAutoLoad", true).toBool();
}

void UserSettings::setVideoSound(bool on)
{
    if (on == getVideoSound())
        return;

    mSettings.setValue("videoSound", on);
    emit videoSoundChanged();
}

bool UserSettings::getVideoSound() const
{
    return mSettings.value("videoSound", true).toBool();
}

void UserSettings::setVideoLoopPlay(bool loop)
{
    if (loop == getVideoLoopPlay())
        return;

    mSettings.setValue("videoLoopPlay", loop);
    emit videoLoopPlayChanged();
}

bool UserSettings::getVideoLoopPlay() const
{
    return mSettings.value("videoLoopPlay", true).toBool();
}

void UserSettings::setGiantEmojis(bool giantEmojis)
{
    if (giantEmojis == getGiantEmojis())
        return;

    mSettings.setValue("giantEmojis", giantEmojis);
    emit giantEmojisChanged();
}

bool UserSettings::getGiantEmojis() const
{
    return mSettings.value("giantEmojis", true).toBool();
}

void UserSettings::setSonglinkEnabled(bool enabled)
{
    if (enabled == getSonglinkEnabled())
        return;

    mSettings.setValue("songlinkEnabled", enabled);
    emit songlinkEnabledChanged();
}

bool UserSettings::getSonglinkEnabled() const
{
    return mSettings.value("songlinkEnabled", true).toBool();
}

void UserSettings::setWrapLabels(bool wrap)
{
    if (wrap == getWrapLabels())
        return;

    mSettings.setValue("wrapLabels", wrap);
    emit wrapLabelsChanged();
}

bool UserSettings::getWrapLabels() const
{
    return mSettings.value("wrapLabels", true).toBool();
}

void UserSettings::setShowFollowsStatus(bool show)
{
    if (show == getShowFollowsStatus())
        return;

    mSettings.setValue("showFollowsStatus", show);
    emit showFollowsActiveStatusChanged();
}

bool UserSettings::getShowFollowsStatus() const
{
    return mSettings.value("showFollowsStatus", true).toBool();
}

void UserSettings::setShowFollowsActiveStatus(bool show)
{
    if (show == getShowFollowsActiveStatus())
        return;

    mSettings.setValue("showFollowsActiveStatus", show);
    emit showFollowsActiveStatusChanged();
}

bool UserSettings::getShowFollowsActiveStatus() const
{
    return mSettings.value("showFollowsActiveStatus", true).toBool();
}

void UserSettings::setShowFeedbackButtons(bool show)
{
    if (show == getShowFeedbackButtons())
        return;

    mSettings.setValue("showFeedbackButtons", show);
    emit showFeedbackButtonsChanged();
}

bool UserSettings::getShowFeedbackButtons() const
{
    return mSettings.value("showFeedbackButtons", true).toBool();
}

void UserSettings::setShowFeedbackNotice(bool show)
{
    mSettings.setValue("showFeedbackNotice", show);
}

bool UserSettings::getShowFeedbackNotice() const
{
    return mSettings.value("showFeedbackNotice", true).toBool();
}

void UserSettings::setRequireAltText(const QString& did, bool require)
{
    mSettings.setValue(key(did, "requireAltText"), require);
}

bool UserSettings::getRequireAltText(const QString& did) const
{
    return mSettings.value(key(did, "requireAltText"), false).toBool();
}

void UserSettings::setScriptRecognition(QEnums::Script script)
{
    const auto oldScript = getScriptRecognition();
    mSettings.setValue("scriptRecognition", (int)script);

    if (script != oldScript)
        emit scriptRecognitionChanged();
}

QEnums::Script UserSettings::getScriptRecognition() const
{
    int script = mSettings.value("scriptRecognition", (int)QEnums::SCRIPT_LATIN).toInt();

    if (script < 0 || script > (int)QEnums::SCRIPT_LAST)
        return QEnums::SCRIPT_LATIN;

    return QEnums::Script(script);
}

QString UserSettings::getMutedRepostsListUri(const QString& did) const
{
    const ATProto::ATUri uri(did, ATProto::ATUri::COLLECTION_GRAPH_LIST, RKEY_MUTED_REPOSTS);
    return uri.toString();
}

void UserSettings::setAutoLinkCard(bool autoLinkCard)
{
    mSettings.setValue("autoLinkCard", autoLinkCard);
}

bool UserSettings::getAutoLinkCard() const
{
    return mSettings.value("autoLinkCard", true).toBool();
}

void UserSettings::setThreadAutoNumber(bool autoNumber)
{
    mSettings.setValue("threadAutoNumber", autoNumber);
}

bool UserSettings::getThreadAutoNumber() const
{
    return mSettings.value("threadAutoNumber", true).toBool();
}

void UserSettings::setThreadPrefix(QString prefix)
{
    mSettings.setValue("threadPrefix", prefix);
}

QString UserSettings::getThreadPrefix() const
{
    return mSettings.value("threadPrefix", UnicodeFonts::THREAD_SYMBOL).toString();
}

void UserSettings::setThreadAutoSplit(bool autoSplit)
{
    mSettings.setValue("threadAutoSplit", autoSplit);
}

bool UserSettings::getThreadAutoSplit() const
{
    return mSettings.value("threadAutoSplit", false).toBool();
}

void UserSettings::setUserHashtags(const QString& did, const QStringList& hashtags)
{
    qDebug() << "Save user hashtags:" << did;
    mSettings.setValue(key(did, "userHashtags"), hashtags);
}

QStringList UserSettings::getUserHashtags(const QString& did) const
{
    return mSettings.value(key(did, "userHashtags")).toStringList();
}

void UserSettings::setSeenHashtags(const QStringList& hashtags)
{
    qDebug() << "Save seen hashtags";
    mSettings.setValue("seenHashtags", hashtags);
}

QStringList UserSettings::getSeenHashtags() const
{
    return mSettings.value("seenHashtags").toStringList();
}

void UserSettings::setOfflineUnread(const QString& did, int unread)
{
    mSettings.setValue(key(did, "offlineUnread"), unread);
}

int UserSettings::getOfflineUnread(const QString& did) const
{
    return mSettings.value(key(did, "offlineUnread"), 0).toInt();
}

void UserSettings::setOfflineMessageCheckTimestamp(QDateTime timestamp)
{
    mSettings.setValue("offlineMessageCheckTimestamp", timestamp);
}

QDateTime UserSettings::getOfflineMessageCheckTimestamp() const
{
    return mSettings.value("offlineMessageCheckTimestamp").toDateTime();
}

void UserSettings::setOffLineChatCheckRev(const QString& did, const QString& rev)
{
    mSettings.setValue(key(did, "offlineChatCheckRev"), rev);
}

QString UserSettings::getOffLineChatCheckRev(const QString& did) const
{
    return mSettings.value(key(did, "offlineChatCheckRev")).toString();
}

void UserSettings::setCheckOfflineChat(const QString& did, bool check)
{
    mSettings.setValue(key(did, "checkOfflineChat"), check);
}

bool UserSettings::mustCheckOfflineChat(const QString& did) const
{
    return mSettings.value(key(did, "checkOfflineChat"), false).toBool();
}

void UserSettings::resetNextNotificationId()
{
    mSettings.setValue("nextNotificationId", 1);
}

int UserSettings::getNextNotificationId()
{
    int id = mSettings.value("nextNotificationId", 1).toInt();
    int nextId = id + 1;
    mSettings.setValue("nextNotificationId", nextId);
    return id;
}

void UserSettings::setNewLabelNotifications(const QString& did, bool enable)
{
    mSettings.setValue(key(did, "newLabelNotifications"), enable);
}

bool UserSettings::getNewLabelNotifications(const QString& did) const
{
    return mSettings.value(key(did, "newLabelNotifications"), true).toBool();
}

void UserSettings::setNotificationsWifiOnly(bool enable)
{
    mSettings.setValue("notificationsWifiOnly", enable);
}

bool UserSettings::getNotificationsWifiOnly() const
{
    return mSettings.value("notificationsWifiOnly", false).toBool();
}

void UserSettings::setNotificationsForAllAccounts(const QString& did, bool enable)
{
    if (enable != getNotificationsForAllAccounts(did))
    {
        mSettings.setValue(key(did, "notificationsForAllAccounts"), enable);
        emit notificationsForAllAccountsChanged();
    }
}

bool UserSettings::getNotificationsForAllAccounts(const QString& did) const
{
    return mSettings.value(key(did, "notificationsForAllAccounts"), true).toBool();
}

bool UserSettings::getShowQuotesWithBlockedPost(const QString& did) const
{
    if (!mShowQuotesWithBlockedPost)
        mShowQuotesWithBlockedPost = mSettings.value(key(did, "showQuotesWithBlockedPost"), true).toBool();

    return *mShowQuotesWithBlockedPost;
}

void UserSettings::setShowQuotesWithBlockedPost(const QString& did, bool show)
{
    mSettings.setValue(key(did, "showQuotesWithBlockedPost"), show);
    mShowQuotesWithBlockedPost = show;
}

bool UserSettings::getShowFollowedReposts(const QString& did) const
{
    if (!mShowFollowedReposts)
        mShowFollowedReposts = mSettings.value(key(did, "showFollowedReposts"), true).toBool();

    return *mShowFollowedReposts;
}

void UserSettings::setShowFollowedReposts(const QString& did, bool show)
{
    mSettings.setValue(key(did, "showFollowedReposts"), show);
    mShowFollowedReposts = show;
}

bool UserSettings::getShowSelfReposts(const QString& did) const
{
    if (!mShowSelfReposts)
        mShowSelfReposts = mSettings.value(key(did, "showSelfReposts"), true).toBool();

    return *mShowSelfReposts;
}

void UserSettings::setShowSelfReposts(const QString& did, bool show)
{
    mSettings.setValue(key(did, "showSelfReposts"), show);
    mShowSelfReposts = show;
}

bool UserSettings::getHideRepliesInThreadFromUnfollowed(const QString did) const
{
    if (!mHideRepliesInThreadFromUnfollowed)
        mHideRepliesInThreadFromUnfollowed = mSettings.value(key(did, "hideRepliesInThreadFromUnfollowed"), false).toBool();

    return *mHideRepliesInThreadFromUnfollowed;
}

void UserSettings::setHideRepliesInThreadFromUnfollowed(const QString& did, bool hide)
{
    mSettings.setValue(key(did, "hideRepliesInThreadFromUnfollowed"), hide);
    mHideRepliesInThreadFromUnfollowed = hide;
}

bool UserSettings::getAssembleThreads(const QString& did) const
{
    if (!mAssembleThreads)
        mAssembleThreads = mSettings.value(key(did, "assembleThreads"), true).toBool();

    return *mAssembleThreads;
}

void UserSettings::setAssembleThreads(const QString& did, bool assemble)
{
    mSettings.setValue(key(did, "assembleThreads"), assemble);
    mAssembleThreads = assemble;
}

QEnums::ReplyOrder UserSettings::getReplyOrder(const QString& did) const
{
    const int replyOrder = mSettings.value(key(did, "replyOrder"), int(QEnums::REPLY_ORDER_SMART)).toInt();

    if (replyOrder < 0 || replyOrder > QEnums::REPLY_ORDER_LAST)
        return QEnums::REPLY_ORDER_SMART;

    return QEnums::ReplyOrder(replyOrder);
}

void UserSettings::setReplyOrder(const QString& did, QEnums::ReplyOrder replyOrder)
{
    mSettings.setValue(key(did, "replyOrder"), int(replyOrder));
}

bool UserSettings::getReplyOrderThreadFirst(const QString& did) const
{
    return mSettings.value(key(did, "replyOrderThreadFirst"), true).toBool();
}

void UserSettings::setReplyOrderThreadFirst(const QString& did, bool threadFirst)
{
    mSettings.setValue(key(did, "replyOrderThreadFirst"), threadFirst);
}

bool UserSettings::getRewindToLastSeenPost(const QString& did) const
{
    return mSettings.value(key(did, "rewindToLastSeenPost"), true).toBool();
}

void UserSettings::setRewindToLastSeenPost(const QString& did, bool rewind)
{
    mSettings.setValue(key(did, "rewindToLastSeenPost"), rewind);
}

QStringList UserSettings::getRecentGifs(const QString& did) const
{
    const QStringList list = mSettings.value(key(did, "recentGifs")).toStringList();
    QStringList gifIdList;

    for (const auto& s : list)
    {
        if (s.isEmpty())
            continue;

        const auto json = QJsonDocument::fromJson(s.toUtf8());
        const ATProto::XJsonObject& xjson(json.object());
        const auto id = xjson.getOptionalString("id");

        if (id)
            gifIdList.push_back(*id);
    }

    return gifIdList;
}

void UserSettings::setRecentGifs(const QString& did, const QStringList& gifIds)
{
    QStringList list;

    // The ID's are stored as JSON objects in older versions we stored all
    // GIF attributes in a JSON object.
    for (const auto& id : gifIds) {
        QJsonObject json;
        json.insert("id", id);
        const QJsonDocument jsonDoc(json);
        list.push_back(jsonDoc.toJson(QJsonDocument::Compact));
    }

    mSettings.setValue(key(did, "recentGifs"), list);
}

QStringList UserSettings::getLastSearches(const QString& did) const
{
    return mSettings.value(key(did, "lastSearches")).toStringList();
}

void UserSettings::setLastSearches(const QString& did, const QStringList& lastSearches)
{
    mSettings.setValue(key(did, "lastSearches"), lastSearches);
}

QStringList UserSettings::getLastProfileSearches(const QString& did) const
{
    return mSettings.value(key(did, "lastProfileSearches")).toStringList();
}

void UserSettings::setLastProfileSearches(const QString& did, const QStringList& lastDids)
{
    mSettings.setValue(key(did, "lastProfileSearches"), lastDids);
}

QEnums::ContentVisibility UserSettings::getSearchAdultOverrideVisibility(const QString& did)
{
    int visibility = mSettings.value(key(did, "searchAdultOverrideVisibility"), (int)QEnums::CONTENT_VISIBILITY_SHOW).toInt();

    if (visibility < 0 || visibility > (int)QEnums::CONTENT_VISIBILITY_LAST)
        return QEnums::CONTENT_VISIBILITY_SHOW;

    return QEnums::ContentVisibility(visibility);
}

void UserSettings::setSearchAdultOverrideVisibility(const QString& did, QEnums::ContentVisibility visibility)
{
    mSettings.setValue(key(did, "searchAdultOverrideVisibility"), (int)visibility);
}

bool UserSettings::getShowTrendingTopics() const
{
    return mSettings.value("showTrendingTopics", true).toBool();
}

void UserSettings::setShowTrendingTopics(bool show)
{
    if (show != getShowTrendingTopics())
    {
        mSettings.setValue("showTrendingTopics", show);
        emit showTrendingTopicsChanged();
    }
}

bool UserSettings::getShowSuggestedFeeds() const
{
    return mSettings.value("showSuggestedFeeds", true).toBool();
}

void UserSettings::setShowSuggestedFeeds(bool show)
{
    if (show != getShowSuggestedFeeds())
    {
        mSettings.setValue("showSuggestedFeeds", show);
        emit showSuggestedFeedsChanged();
    }
}

bool UserSettings::getShowSuggestedUsers() const
{
    return mSettings.value("showSuggestedUsers", true).toBool();
}

void UserSettings::setShowSuggestedUsers(bool show)
{
    if (show != getShowSuggestedUsers())
    {
        mSettings.setValue("showSuggestedUsers", show);
        emit showSuggestedUsersChanged();
    }
}

bool UserSettings::getShowSuggestedStarterPacks() const
{
    return mSettings.value("showSuggestedStarterPacks", true).toBool();
}

void UserSettings::setShowSuggestedStarterPacks(bool show)
{
    if (show != getShowSuggestedStarterPacks())
    {
        mSettings.setValue("showSuggestedStarterPacks", show);
        emit showSuggestedStarterPacksChanged();
    }
}

QString UserSettings::getDefaultPostLanguage(const QString& did) const
{
    return mSettings.value(key(did, "defaultPostLanguage")).toString();
}

void UserSettings::setDefaultPostLanguage(const QString& did, const QString& language)
{
    mSettings.setValue(key(did, "defaultPostLanguage"), language);
}

QStringList UserSettings::getUsedPostLanguages(const QString& did) const
{
    return mSettings.value(key(did, "usedPostLanguages")).toStringList();
}

void UserSettings::setUsedPostLanguages(const QString& did, const QStringList& languages)
{
    mSettings.setValue(key(did, "usedPostLanguages"), languages);
}

QStringList UserSettings::getContentLanguages(const QString& did) const
{
    if (!mContentLanguages)
        mContentLanguages = mSettings.value(key(did, "contentLanguages")).toStringList();

    return *mContentLanguages;
}

void UserSettings::setContentLanguages(const QString& did, const QStringList& languages)
{
    const auto oldLangs = getContentLanguages(did);
    QStringList sortedLangs = languages;
    std::sort(sortedLangs.begin(), sortedLangs.end());

    if (oldLangs != sortedLangs)
    {
        mSettings.setValue(key(did, "contentLanguages"), sortedLangs);
        mContentLanguages = sortedLangs;
        emit contentLanguageFilterChanged();
    }
}

QStringList UserSettings::getExcludeDetectLanguages(const QString& did) const
{
    return mSettings.value(key(did, "excludeDetectLanguages")).toStringList();
}

void UserSettings::setExcludeDetectLanguages(const QString& did, const QStringList& languages)
{
    const auto oldLangs = getExcludeDetectLanguages(did);
    QStringList sortedLangs = languages;
    std::sort(sortedLangs.begin(), sortedLangs.end());

    if (oldLangs != sortedLangs)
        mSettings.setValue(key(did, "excludeDetectLanguages"), sortedLangs);
}

bool UserSettings::getShowUnknownContentLanguage(const QString& did) const
{
    if (!mShowUnknownContentLanguage)
        mShowUnknownContentLanguage = mSettings.value(key(did, "showUnknownContentLanguage"), true).toBool();

    return *mShowUnknownContentLanguage;
}

void UserSettings::setShowUnknownContentLanguage(const QString& did, bool show)
{
    const bool oldShow = getShowUnknownContentLanguage(did);

    if (oldShow != show)
    {
        mSettings.setValue(key(did, "showUnknownContentLanguage"), show);
        mShowUnknownContentLanguage = show;
        emit contentLanguageFilterChanged();
    }
}

bool UserSettings::getDefaultLanguageNoticeSeen() const
{
    return mSettings.value("defaultLanguageNoticeSeen", false).toBool();
}

void UserSettings::setDefautlLanguageNoticeSeen(bool seen)
{
    mSettings.setValue("defaultLanguageNoticeSeen", seen);
}

bool UserSettings::getShowLanguageTags() const
{
    return mSettings.value("showLanguageTags", false).toBool();
}

void UserSettings::setShowLanguageTags(bool show)
{
    mSettings.setValue("showLanguageTags", show);
}

QDate UserSettings::getAnniversaryNoticeDate(const QString& did) const
{
    return mSettings.value(key(did, "anniversaryNoticeDate")).toDate();
}

void UserSettings::setAnniversaryNoticeDate(const QString& did, QDate date)
{
    mSettings.setValue(key(did, "anniversaryNoticeDate"), date);
}

const QJsonDocument UserSettings::getFocusHashtags(const QString& did) const
{
    return mSettings.value(key(did, "focusHashtags")).toJsonDocument();
}

void UserSettings::setFocusHashtags(const QString& did, const QJsonDocument& jsonHashtags)
{
    mSettings.setValue(key(did, "focusHashtags"), jsonHashtags);
}

QStringList UserSettings::getLabels(const QString& did, const QString& labelerDid) const
{
    return mSettings.value(labelsKey(did, labelerDid)).toStringList();
}

void UserSettings::setLabels(const QString& did, const QString& labelerDid, const QStringList labels)
{
    mSettings.setValue(labelsKey(did, labelerDid), labels);
}

void UserSettings::removeLabels(const QString& did, const QString& labelerDid)
{
    mSettings.remove(labelsKey(did, labelerDid));
}

bool UserSettings::containsLabeler(const QString& did, const QString& labelerDid) const
{
    return mSettings.contains(labelsKey(did, labelerDid));
}

bool UserSettings::getFixedLabelerEnabled(const QString& did, const QString& labelerDid) const
{
    return mSettings.value(fixedLabelerKey(did, labelerDid), true).toBool();
}

void UserSettings::setFixedLabelerEnabled(const QString& did, const QString& labelerDid, bool enabled)
{
    mSettings.setValue(fixedLabelerKey(did, labelerDid), enabled);
}

SearchFeed::List UserSettings::getPinnedSearchFeeds(const QString& did) const
{
    SearchFeed::List searchFeeds;
    const auto jsonArray = mSettings.value(key(did, "pinnedSearchFeeds")).toJsonArray();

    for (const auto& json : jsonArray)
    {
        try {
            const auto searchFeed = SearchFeed::fromJson(json.toObject());
            searchFeeds.push_back(searchFeed);
        } catch (ATProto::InvalidJsonException& e) {
            qWarning() << "Invalid pinned search feed:" << e.msg();
        }
    }

    return searchFeeds;
}

void UserSettings::setPinnedSearchFeeds(const QString& did, const SearchFeed::List& searchFeeds)
{
    QJsonArray jsonArray;

    for (const auto& searchFeed : searchFeeds)
    {
        const QJsonObject json = searchFeed.toJson();
        jsonArray.push_back(json);
    }

    mSettings.setValue(key(did, "pinnedSearchFeeds"), jsonArray);
}

void UserSettings::cleanup()
{
    mSettings.remove("draftRepoToFileMigration");
    mSettings.remove("floatingNavButtons");
    mSettings.remove("landscapeSideBar");
}

}
