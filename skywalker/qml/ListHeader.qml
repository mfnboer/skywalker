import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    property string userDid
    required property listviewbasic list
    property Skywalker skywalker: root.getSkywalker(userDid)

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

    AccessibleText {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        elide: Text.ElideRight
        font.bold: true
        text: (skywalker.favoriteFeeds.homeFeedUri === list.key ? "🏠 " : "") + list.name
    }
}
