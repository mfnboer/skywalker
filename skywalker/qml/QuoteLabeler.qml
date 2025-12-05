import QtQuick
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
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
            userDid: quoteColumn.userDid
            author: labeler.creator

            onClicked: skywalker.getDetailedProfile(labeler.creator.did)
        }

        Column {
            Layout.fillWidth: true

            AuthorNameAndStatus {
                width: parent.width
                userDid: quoteColumn.userDid
                author: labeler.creator
            }

            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: qsTr(`feed by @${labeler.creator.handle}`)

                Accessible.ignored: true
            }
        }
    }

    AccessibleText {
        width: parent.width - 20
        wrapMode: Text.Wrap
        maximumLineCount: 5
        elide: Text.ElideRight
        text: labeler.creator.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

}
