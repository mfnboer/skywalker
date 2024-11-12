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
            Layout.preferredWidth: 34
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
                color: guiSettings.textColor
                plainText: labeler.creator.name

                Accessible.ignored: true
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
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
        color: guiSettings.textColor
        text: labeler.creator.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

}
