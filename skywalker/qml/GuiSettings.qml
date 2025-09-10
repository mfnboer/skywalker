﻿import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

// I would like this as singleton. However the properties do not update
// when Material.theme changes in a singleton. (Bug in Qt??)
Item {
    property Skywalker skywalker: root.getSkywalker()
    readonly property var userSettings: skywalker ? skywalker.getUserSettings() : null
    property bool isLightMode: Material.theme === Material.Light

    // Android geometry
    property int footerMargin: getNavigationBarSize(QEnums.INSETS_SIDE_BOTTOM)
    property int headerMargin: getStatusBarSize(QEnums.INSETS_SIDE_TOP)
    property int leftMargin: Math.max(getNavigationBarSize(QEnums.INSETS_SIDE_LEFT), getDisplayCutoutSize(QEnums.INSETS_SIDE_LEFT))
    property int rightMargin: Math.max(getNavigationBarSize(QEnums.INSETS_SIDE_RIGHT), getDisplayCutoutSize(QEnums.INSETS_SIDE_RIGHT))

    function updateScreenMargins() {
        footerMargin = getNavigationBarSize(QEnums.INSETS_SIDE_BOTTOM)
        headerMargin = getStatusBarSize(QEnums.INSETS_SIDE_TOP)
        leftMargin = Math.max(getNavigationBarSize(QEnums.INSETS_SIDE_LEFT), getDisplayCutoutSize(QEnums.INSETS_SIDE_LEFT))
        rightMargin = Math.max(getNavigationBarSize(QEnums.INSETS_SIDE_RIGHT), getDisplayCutoutSize(QEnums.INSETS_SIDE_RIGHT))

        console.debug("MARGINS L:", leftMargin, "R:", rightMargin, "T:", headerMargin, "B:", footerMargin)
    }

    // Geometry
    readonly property int footerHeight: 50 + footerMargin
    readonly property int footerZLevel: 10
    readonly property int headerHeight: 50 + headerMargin
    readonly property int headerZLevel: 10
    readonly property int labelHeight: labelFontHeight + 2
    readonly property int labelRowPadding: 5
    readonly property int maxImageHeight: root.height - headerHeight
    readonly property int sideBarHeaderHeight: 44
    readonly property int sideBarMinWidth: 175
    readonly property int sideBarMaxWidth: 325
    readonly property int statsHeight: appFontHeight + 4
    readonly property int tabBarHeight: 40
    readonly property int threadColumnWidth: 60
    readonly property int threadLineWidth: 2
    readonly property int verificationBadgeSize: 13

    // Aspect ratio (height / width)
    readonly property double videoPreviewRatio: 2160.0 / 3840.0

    // Colors
    readonly property string accentColor: userSettings ? userSettings.accentColor : (isLightMode ? "blue" : "#58a6ff")
    readonly property string activeBorderColor: isLightMode ? "#006600" : "#009900"
    readonly property string activeColor: isLightMode ? "#009900" : "#006600"
    readonly property string avatarDefaultColor: accentColor
    property string backgroundColor: userSettings ? userSettings.backgroundColor : Material.background
    readonly property string badgeBorderColor: backgroundColor
    readonly property string badgeColor: accentColor
    readonly property string badgeTextColor: "white"
    readonly property string bannerDefaultColor: accentColor
    readonly property string borderColor: isLightMode ? Qt.darker(backgroundColor, 1.1) : Qt.lighter(backgroundColor, 1.6)
    readonly property string borderHighLightColor: isLightMode ? Qt.darker(borderColor, 1.1) : Qt.lighter(borderColor, 1.6)
    readonly property string buttonColor: accentColor
    readonly property string buttonFlatColor: isLightMode ? Qt.lighter(buttonColor, 1.8) : Qt.darker(buttonColor, 1.8)
    readonly property string buttonNeutralColor: isLightMode ? Qt.darker(backgroundColor, 1.07) : Qt.lighter(backgroundColor, 1.6)
    readonly property string buttonTextColor: "white"
    readonly property string contentLabelColor: isLightMode ? "#f3f3f3" : "#1d3030"
    readonly property string contentUserLabelColor: isLightMode ? "lightblue" : "steelblue"
    readonly property string disabledColor: isLightMode ? "lightgrey" : "darkslategrey"
    readonly property string dragHighLightColor: isLightMode ? Qt.darker(backgroundColor, 1.1) : Qt.lighter(backgroundColor, 1.6)
    readonly property string errorColor: isLightMode ? "darkred" : "palevioletred"
    readonly property string favoriteColor: "gold"
    readonly property string fullScreenColor: "black"
    readonly property string footerColor: backgroundColor
    readonly property string handleColor: Material.color(Material.Grey)
    readonly property string headerColor: backgroundColor
    readonly property string headerHighLightColor: isLightMode ? "lightblue" : "darkslategrey"
    readonly property string headerTextColor: textColor
    readonly property string labelColor: isLightMode ? "lightblue" : "steelblue"
    readonly property string likeColor: "palevioletred"
    readonly property string linkColorDarkMode: "#58a6ff"
    property string linkColor: userSettings ? userSettings.linkColor : (isLightMode ? "blue" : linkColorDarkMode)
    readonly property string liveColor: "red"
    readonly property string liveTextColor: "white"
    readonly property string menuColor: backgroundColor
    readonly property string messageTimeColor: Material.color(Material.Grey)
    readonly property string messageNewBackgroundColor: isLightMode ? "#f3f3f3" : "#1d3030"
    readonly property string messageNewTextColor: isLightMode ? "black" : "white"
    readonly property string messageUserBackgroundColor: isLightMode ? "#58a6ff" : "#0053b3"
    readonly property string messageUserTextColor: "white"
    readonly property string messageOtherBackgroundColor: isLightMode ? "#f3f3f3" : "#1d3030"
    readonly property string messageOtherTextColor: isLightMode ? "black" : "white"
    readonly property string moderatorIconColor: "lightgrey"
    readonly property string videoIconColor: "lightgrey"
    readonly property string placeholderTextColor: Material.color(Material.Grey)
    readonly property string postHighLightColor: isLightMode ? Qt.darker(backgroundColor, 1.1) : Qt.lighter(backgroundColor, 1.6)
    readonly property string selectionColor: accentColor
    readonly property string separatorColor: isLightMode ? Qt.darker(backgroundColor, 1.08) : Qt.lighter(backgroundColor, 1.6)
    readonly property string separatorHighLightColor: isLightMode ? Qt.darker(separatorColor, 1.1) : Qt.lighter(separatorColor, 1.6)
    readonly property string settingsHighLightColor: isLightMode ? Qt.darker(backgroundColor, 1.05) : Qt.lighter(backgroundColor, 1.4)
    readonly property string sideBarColor: isLightMode ? Qt.darker(backgroundColor, 1.01) : Qt.lighter(backgroundColor, 1.2)
    readonly property string skywalkerLogoColor: "#0387c7"
    readonly property string starterpackColor: accentColor
    readonly property string statsColor: Material.color(Material.Grey)
    property string textColor: Material.foreground
    readonly property string textInputBackgroundColor: isLightMode ? "#f3f3f3" : "#1d3030"
    readonly property string textLengthExceededColor: "palevioletred"

    // Opacity
    readonly property double focusHighlightOpacity: 0.2

    // Font size
    readonly property double appFontHeight: fontMetrics.height
    readonly property double labelFontHeight: appFontHeight * 6/8
    readonly property double labelFontSize: scaledFont(6/8)

    // Misc
    readonly property real flickDeceleration: 700
    readonly property real maxFlickVelocity: 25000
    readonly property bool flickPixelAligned: false
    readonly property bool isAndroid: Qt.platform.os === "android"

    // Identity
    readonly property string skywalkerHandle: "@skywalker.thereforeiam.eu"
    readonly property string blueskyTrendingDid: "did:plc:qrz3lhbyuxbeilrc6nekdqme"

    id: guiItem

    Accessible.ignored: true

    FontMetrics {
        id: fontMetrics
        font: Application.font
    }

    function createComponent(qmlFileName) {
        let component = Qt.createComponent(qmlFileName)

        if (component.status === Component.Error)
            console.warn(component.errorString())

        return component
    }

    function scaledFont(scaleFactor) {
        return Application.font.pointSize * scaleFactor;
    }

    function isToday(date) {
        const today = new Date()
        return date.getDate() === today.getDate() &&
            date.getMonth() === today.getMonth() &&
            date.getFullYear() === today.getFullYear()
    }

    function isYesterday(date) {
        const nextDay = new Date(new Date().setDate(date.getDate() + 1))
        return isToday(nextDay)
    }

    function isTomorrow(date) {
        const prevDay = new Date(new Date().setDate(date.getDate() - 1))
        return isToday(prevDay)
    }

    function durationToString(duration) {
        if (duration < 59.5)
            return Math.round(duration) + qsTr("s", "seconds")

        duration = duration / 60
        if (duration < 59.5)
            return Math.round(duration) + qsTr("m", "minutes")

        duration = duration / 60
        if (duration < 23.5)
            return Math.round(duration) + qsTr("h", "hours")

        duration = duration / 24
        if (duration < 30.4368499)
            return Math.round(duration) + qsTr("d", "days")

        duration = duration / 30.4368499
        if (duration < 35.5)
            return Math.round(duration) + qsTr("mo", "months")

        duration = duration / 12
        return Math.round(duration) + qsTr("yr", "years")
    }

    function videoDurationToString(durationMs) {
        const minutes = Math.floor(durationMs / 60000)
        const seconds = String(Math.ceil((durationMs - minutes * 60000) / 1000)).padStart(2, '0')
        return `${minutes}:${seconds}`
    }

    function shortFormatDateTime(date) {
        return date.toLocaleString(Qt.locale(), Locale.ShortFormat)
    }

    function dateTimeIndication(date) {
        if (isNaN(date.getTime()))
            return ""

        if (isToday(date))
            return Qt.locale().toString(date, Qt.locale().timeFormat(Locale.ShortFormat))

        if (isYesterday(date))
            return qsTr(`yesterday ${Qt.locale().toString(date, Qt.locale().timeFormat(Locale.ShortFormat))}`)

        return shortFormatDateTime(date)
    }

    function expiresIndication(expiresAt) {
        if (isToday(expiresAt))
            return Qt.locale().toString(expiresAt, Qt.locale().timeFormat(Locale.ShortFormat))

        if (isTomorrow(expiresAt))
            return qsTr(`tomorrow ${Qt.locale().toString(expiresAt, Qt.locale().timeFormat(Locale.ShortFormat))}`)

        return shortFormatDateTime(expiresAt)
    }

    function askDiscardSaveQuestion(parent, question, onDiscardCb, onSaveCb) {
        let component = guiSettings.createComponent("Message.qml")
        let message = component.createObject(parent, { standardButtons: Dialog.No | Dialog.Discard | Dialog.Save })
        message.onDiscarded.connect(() => { message.destroy(); onDiscardCb() })
        message.onAccepted.connect(() => { message.destroy(); onSaveCb() })
        message.onRejected.connect(() => message.destroy())
        message.show(question)
    }

    function askYesNoQuestion(parent, question, onYesCb, onNoCb = () => {}) {
        let component = guiSettings.createComponent("Message.qml")
        let message = component.createObject(parent, { standardButtons: Dialog.Yes | Dialog.No })
        message.onAccepted.connect(() => { message.destroy(); onYesCb() })
        message.onRejected.connect(() => { message.destroy(); onNoCb() })
        message.show(question)
    }

    function noticeOkCancel(parent, question, onOkCb, onCancelCb = () => {}) {
        let component = guiSettings.createComponent("Message.qml")
        let message = component.createObject(parent, { standardButtons: Dialog.Ok | Dialog.Cancel })
        message.onAccepted.connect(() => { message.destroy(); onOkCb() })
        message.onRejected.connect(() => { message.destroy(); onCancelCb() })
        message.show(question)
    }

    function notice(parent, msg, emoji = "", onOkCb = () => {}) {
        let component = guiSettings.createComponent("Message.qml")
        let message = component.createObject(parent, { emoji: emoji, standardButtons: Dialog.Ok })
        message.onAccepted.connect(() => { message.destroy(); onOkCb() })
        message.onRejected.connect(() => message.destroy())
        message.show(msg)
    }

    function askConvertGif(parent, gifSource, onVideoCb, onImageCb) {
        let component = guiSettings.createComponent("ConvertGifDialog.qml")
        let dialog = component.createObject(parent, { gifSource: gifSource })
        dialog.onAccepted.connect(() => { dialog.destroy(); onVideoCb() })
        dialog.onRejected.connect(() => { dialog.destroy(); onImageCb() })
        dialog.open()
    }

    function showProgress(parent, msg, onCancelCb) {
        let component = guiSettings.createComponent("ProgressDialog.qml")
        let dialog = component.createObject(parent)
        dialog.onRejected.connect(() => { dialog.destroy(); onCancelCb() })
        dialog.show(msg)
        return dialog
    }

    function toWordSequence(stringList) {
        if (!stringList)
            return ""

        let wordSequence = stringList[0]

        for (let i = 1; i < stringList.length - 1; ++i)
            wordSequence += ", " + stringList[i]

        if (stringList.length > 1) {
            wordSequence += qsTr(" and ")
            wordSequence += stringList[stringList.length - 1]
        }

        return wordSequence
    }

    function listTypeName(purpose) {
        switch (purpose) {
        case QEnums.LIST_PURPOSE_MOD:
            return qsTr("Moderation List")
        case QEnums.LIST_PURPOSE_CURATE:
            return qsTr("User List")
        case QEnums.LIST_PURPOSE_REFERENCE:
            return qsTr("Reference List")
        default:
            return qsTr("List")
        }
    }

    function getFocusHashtagEntryText(entry) {
        let text = ""

        for (let i = 0; i < entry.hashtags.length; ++i) {
            const tag = entry.hashtags[i]

            if (i > 0)
                text += ' '

            text += `<a href="${tag}" style="color: ${guiSettings.linkColor}; text-decoration: none">#${tag}</a>`
        }

        console.debug("TEXT:", text)
        return text
    }

    function feedDefaultAvatar(feed: generatorview) : SvgImage {
        return feed.creator.did === guiSettings.blueskyTrendingDid ? SvgOutline.trending : SvgFilled.feed
    }

    function favoriteDefaultAvatar(favorite: favoritefeedview) : SvgImage {
        if (favorite.isNull())
            return SvgFilled.home

        switch (favorite.type) {
        case QEnums.FAVORITE_FEED:
            return feedDefaultAvatar(favorite.generatorView)
        case QEnums.FAVORITE_LIST:
            return SvgFilled.list
        case QEnums.FAVORITE_SEARCH:
            return favorite.searchFeed.isHashtag() ? SvgOutline.hashtag : SvgOutline.search
        }

        return SvgOutline.feed
    }

    function contentVisible(author)
    {
        if (author.viewer.blockedBy)
            return false

        let visibility = skywalker.getContentVisibility(author.labels)
        return visibility === QEnums.CONTENT_VISIBILITY_SHOW
    }

    function feedContentVisible(feed)
    {
        let visibility = skywalker.getContentVisibility(feed.labels)
        return visibility === QEnums.CONTENT_VISIBILITY_SHOW
    }

    function filterContentLabelsToShow(contentLabels) {
        let contentFilter = skywalker.getContentFilter()
        let labels = []

        for (let i = 0; i < contentLabels.length; ++i) {
            const label = contentLabels[i]

            if (!label.isSystemLabel() && contentFilter.mustShowBadge(label))
                labels.push(label)
        }

        return labels
    }

    function getFilteredPostsFooterText(model) {
        if (!model)
            return ""

        if (!model.isFilterModel())
            return ""

        if (model.numPostsChecked === 0)
            return qsTr(`No more '${model.feedName}' posts in feed till ${model.checkedTillTimestamp.toLocaleString(Qt.locale(), Locale.ShortFormat)}`)

        return qsTr(`No more '${model.feedName}' posts in ${model.numPostsChecked} feed posts till ${model.checkedTillTimestamp.toLocaleString(Qt.locale(), Locale.ShortFormat)}`)
    }

    function getContentModeSvg(contentMode) {
        switch (contentMode) {
        case QEnums.CONTENT_MODE_VIDEO:
            return SvgOutline.film
        case QEnums.CONTENT_MODE_MEDIA:
            return SvgOutline.image
        case QEnums.CONTENT_MODE_MEDIA_TILES:
            return SvgOutline.gallery
        case QEnums.CONTENT_MODE_VIDEO_TILES:
            return SvgOutline.videoGallery
        default:
            return SvgOutline.chat
        }
    }

    function isUserDid(did) {
        return skywalker.getUserDid() === did
    }

    function isUser(author) {
        return isUserDid(author.did)
    }

    function threadStartColor(color) {
        return color
    }

    function threadMidColor(color) {
        return isLightMode ?  Qt.lighter(threadStartColor(color), 1.3) : Qt.darker(threadStartColor(color), 1.3)
    }

    function threadEndColor(color) {
        return Material.background
    }

    function threadEntryColor(color) {
        return isLightMode ? Qt.darker(threadStartColor(color), 1.2) : Qt.lighter(threadStartColor(color), 1.2)
    }

    function forbiddenBackgroundColors() {
        return [textColor,
                badgeColor,
                buttonColor,
                contentLabelColor,
                contentUserLabelColor,
                handleColor,
                likeColor,
                linkColor,
                messageTimeColor,
                messageNewBackgroundColor,
                messageUserBackgroundColor,
                messageOtherBackgroundColor,
                statsColor]
    }

    function forbiddenAccentColors() {
        return [backgroundColor,
                buttonTextColor,
                headerColor]
    }

    function forbiddenLinkColors() {
        return [backgroundColor,
                messageNewBackgroundColor,
                messageUserBackgroundColor,
                messageOtherBackgroundColor,
                postHighLightColor]
    }

    function getNavigationBarSize(side) {
        return displayUtils.getNavigationBarSize(side) / Screen.devicePixelRatio
    }

    function getStatusBarSize(side) {
        return displayUtils.getStatusBarSize(side) / Screen.devicePixelRatio
    }

    function getDisplayCutoutSize(side) {
        return displayUtils.getDisplayCutoutSize(side) / Screen.devicePixelRatio
    }
}
