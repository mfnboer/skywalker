import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    property string userDid
    required property listviewbasic list

    id: listHeader
    spacing: 10

    ListAvatar {
        id: avatar
        Layout.preferredWidth: 34
        Layout.preferredHeight: 34
        userDid: listHeader.userDid
        avatarUrl: list.avatarThumb

        onClicked: root.viewListByUri(list.uri, false)
    }

    SkyCleanedTextLine {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        elide: Text.ElideRight
        font.bold: true
        plainText: list.name
    }
}
