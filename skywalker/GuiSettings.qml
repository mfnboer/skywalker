import QtQuick
import QtQuick.Controls
import skywalker

Item {
    // Geometry
    readonly property int footerHeight: 50
    readonly property int footerZLevel: 10
    readonly property int headerHeight: 50
    readonly property int headerZLevel: 10
    readonly property int threadBarWidth: 12 // In 5px units
    readonly property int threadColumnWidth: threadBarWidth * 5

    // Colors
    readonly property string accentColor: Material.theme === Material.Light ? "blue" : "#58a6ff"
    readonly property string avatarDefaultColor: "blue"
    readonly property string backgroundColor: Material.background
    readonly property string badgeBorderColor: Material.background
    readonly property string badgeColor: Material.theme === Material.Light ? "blue" : "#58a6ff"
    readonly property string badgeTextColor: "white"
    readonly property string bannerDefaultColor: "blue"
    readonly property string borderColor: Material.color(Material.Grey)
    readonly property string buttonColor: Material.theme === Material.Light ? "blue" : "#58a6ff"
    readonly property string buttonNeutralColor: Material.theme === Material.Light ? Material.background : "darkslategrey"
    readonly property string buttonTextColor: "white"
    readonly property string contentLabelColor: Material.theme === Material.Light ? "lightgrey" : "darkslategrey"
    readonly property string contentUserLabelColor: Material.theme === Material.Light ? "lightblue" : "steelblue"
    readonly property string disabledColor: Material.theme === Material.Light ? "lightgrey" : "darkslategrey"
    readonly property string errorColor: Material.theme === Material.Light ? "darkred" : "palevioletred"
    readonly property string favoriteColor: "gold"
    readonly property string footerColor: Material.background
    readonly property string handleColor: Material.color(Material.Grey)
    readonly property string headerColor: "black"
    readonly property string headerHighLightColor: Material.theme === Material.Light ? "lightblue" : "darkslategrey"
    readonly property string headerTextColor: "white"
    readonly property string labelColor: Material.theme === Material.Light ? "lightblue" : "steelblue"
    readonly property string likeColor: "palevioletred"
    readonly property string linkColor: Material.theme === Material.Light ? "blue" : "#58a6ff"
    readonly property string messageTimeColor: Material.color(Material.Grey)
    readonly property string messageUserBackgroundColor: Material.theme === Material.Light ? "#58a6ff" : "#264040"
    readonly property string messageUserTextColor: "white"
    readonly property string messageOtherBackgroundColor: Material.theme === Material.Light ? "#f3f3f3" : "darkslategrey"
    readonly property string messageOtherTextColor: Material.theme === Material.Light ? "black" : "white"
    readonly property string placeholderTextColor: Material.color(Material.Grey)
    readonly property string postHighLightColor: Material.theme === Material.Light ? "aliceblue" : "#264040"
    readonly property string selectionColor: Material.theme === Material.Light ? "blue" : "#58a6ff"
    readonly property string separatorColor: Material.theme === Material.Light ? "lightgrey" : "darkslategrey"
    readonly property string skywalkerLogoColor: "#0387c7"
    readonly property string statsColor: Material.color(Material.Grey)
    readonly property string textColor: Material.foreground
    readonly property string textLengthExceededColor: "palevioletred"
    readonly property string threadEndColor: Material.background
    readonly property string threadEntryColor: Material.theme === Material.Light ? "darkcyan" : "teal"
    readonly property string threadMidColor: Material.theme === Material.Light ? "lightcyan" : "#000033"
    readonly property string threadStartColor: Material.theme === Material.Light ? "cyan" : "navy"

    // Font size
    readonly property double labelFontSize: scaledFont(6/8)

    // Misc
    readonly property real flickDeceleration: 1000

    // Identity
    readonly property string skywalkerHandle: "@skywalker.thereforeiam.eu"

    Accessible.ignored: true

    function scaledFont(scaleFactor) {
        return Application.font.pointSize * scaleFactor;
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

    function askDiscardSaveQuestion(parent, question, onDiscardCb, onSaveCb) {
        let component = Qt.createComponent("Message.qml")
        let message = component.createObject(parent, { standardButtons: Dialog.No | Dialog.Discard | Dialog.Save })
        message.onDiscarded.connect(() => { message.destroy(); onDiscardCb() })
        message.onAccepted.connect(() => { message.destroy(); onSaveCb() })
        message.onRejected.connect(() => message.destroy())
        message.show(question)
    }

    function askYesNoQuestion(parent, question, onYesCb, onNoCb = () => {}) {
        let component = Qt.createComponent("Message.qml")
        let message = component.createObject(parent, { standardButtons: Dialog.Yes | Dialog.No })
        message.onAccepted.connect(() => { message.destroy(); onYesCb() })
        message.onRejected.connect(() => { message.destroy(); onNoCb() })
        message.show(question)
    }

    function notice(parent, msg, emoji = "", onOkCb = () => {}) {
        let component = Qt.createComponent("Message.qml")
        let message = component.createObject(parent, { emoji: emoji, standardButtons: Dialog.Ok })
        message.onAccepted.connect(() => { message.destroy(); onOkCb() })
        message.onRejected.connect(() => message.destroy())
        message.show(msg)
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
        default:
            return qsTr("List")
        }
    }

    function authorVisible(author)
    {
        let visibility = skywalker.getContentVisibility(author.labels)
        return visibility === QEnums.CONTENT_VISIBILITY_SHOW
    }
}
