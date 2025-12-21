import QtQuick
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    property starterpackviewbasic starterPack
    readonly property int margin: 10

    id: quoteColumn
    padding: 10

    Accessible.role: Accessible.StaticText
    Accessible.name: accessibilityUtils.getStarterPackSpeech(starterPack)

    GridLayout {
        columns: 2
        rowSpacing: 0
        width: parent.width

        Rectangle {
            Layout.rowSpan: 2
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            color: "transparent"

            SkySvg {
                width: parent.width
                height: parent.height
                color: guiSettings.starterpackColor
                svg: SvgOutline.starterpack
            }
        }

        SkyCleanedTextLine {
            Layout.fillWidth: true
            Layout.rightMargin: margin
            elide: Text.ElideRight
            font.bold: true
            color: guiSettings.textColor
            plainText: starterPack.name
        }

        AccessibleText {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: margin
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: qsTr(`Starter pack by @${starterPack.creator.handle}`)
        }
    }

    ContentLabels {
        id: contentLabels
        anchors.left: parent.left
        anchors.leftMargin: margin
        anchors.right: undefined
        userDid: quoteColumn.userDid
        contentLabels: starterPack.labels
        contentAuthor: starterPack.creator
    }

    SkyCleanedText {
        topPadding: 10
        width: parent.width - 2 * margin
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        maximumLineCount: 5
        color: guiSettings.textColor
        plainText: starterPack.description
        visible: starterPack.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

}
