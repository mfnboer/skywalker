import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    property externalview postExternal

    width: parent.width
    height: card.columnHeight

    LinkCardView {
        id: card
        anchors.fill: parent
        uri: postExternal.uri
        title: postExternal.title
        description: postExternal.description
        thumbUrl: postExternal.thumbUrl
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.openLink(postExternal.uri)
    }
}
