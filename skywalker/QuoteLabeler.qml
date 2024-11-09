import QtQuick
import QtQuick.Layouts
import skywalker

Column {
    property labelerview labeler

    id: quoteColumn
    padding: 10

    Accessible.role: Accessible.StaticText
    Accessible.name: accessibilityUtils.getLabelerSpeech(labeler)

    RowLayout {
        width: parent.width - 20

        Avatar {
            id: avatar
            width: 34
            Layout.alignment: Qt.AlignTop
            author: labeler.creator

            onClicked: skywalker.getDetailedProfile(labeler.creator.did)
        }

        Column {
            Layout.fillWidth: true

            SkyCleanedTextLine {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: GuiSettings.textColor
                plainText: labeler.creator.name

                Accessible.ignored: true
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: GuiSettings.scaledFont(7/8)
                color: GuiSettings.handleColor
                text: qsTr(`feed by @${labeler.creator.handle}`)

                Accessible.ignored: true
            }
        }
    }

    Text {
        width: parent.width - 20
        wrapMode: Text.Wrap
        maximumLineCount: 5
        elide: Text.ElideRight
        color: GuiSettings.textColor
        text: labeler.creator.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

}
