import QtQuick
import skywalker

Column {
    property starterpackviewbasic starterPack
    readonly property int margin: 10

    id: quoteColumn
    padding: 10

    Accessible.role: Accessible.StaticText
    Accessible.name: accessibilityUtils.getStarterPackSpeech(starterPack)

    SkyCleanedText {
        width: parent.width - 2 * margin
        elide: Text.ElideRight
        font.bold: true
        color: guiSettings.textColor
        plainText: starterPack.name
    }

    AccessibleText {
        width: parent.width - 2 * margin
        elide: Text.ElideRight
        font.pointSize: guiSettings.scaledFont(7/8)
        color: guiSettings.handleColor
        text: qsTr(`Starter pack by @${starterPack.creator.handle}`)
    }

    ContentLabels {
        id: contentLabels
        anchors.left: parent.left
        anchors.leftMargin: margin
        anchors.right: undefined
        contentLabels: starterPack.labels
        contentAuthorDid: starterPack.creator.did
    }

    SkyCleanedText {
        topPadding: 10
        width: parent.width - 2 * margin
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        maximumLineCount: maxTextLines
        color: guiSettings.textColor
        plainText: starterPack.description
        visible: starterPack.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    GuiSettings {
        id: guiSettings
    }
}
