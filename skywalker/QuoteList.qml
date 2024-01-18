import QtQuick
import QtQuick.Layouts
import skywalker

Column {
    property listview list

    id: quoteColumn
    padding: 10

    RowLayout {
        width: parent.width - 20

        ListAvatar {
            id: avatar
            width: 34
            Layout.alignment: Qt.AlignTop
            avatarUrl: list.avatar

            onClicked: root.viewListByUri(list.uri, false)
        }

        Column {
            Layout.fillWidth: true

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: list.name
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: qsTr(`${(guiSettings.listTypeName(list.purpose))} by @${list.creator.handle}`)
            }
        }
    }

    Text {
        width: parent.width - 20
        wrapMode: Text.Wrap
        maximumLineCount: 5
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: guiSettings.textColor
        text: list.description
    }

    GuiSettings {
        id: guiSettings
    }
}
