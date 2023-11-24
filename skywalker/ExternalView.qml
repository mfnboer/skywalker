import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property externalview postExternal

    id: view
    width: parent.width
    height: filter.imageVisible() ? card.columnHeight : filter.height

    LinkCardView {
        id: card
        anchors.fill: parent
        uri: postExternal.uri
        title: postExternal.title
        description: postExternal.description
        thumbUrl: postExternal.thumbUrl
        visible: filter.imageVisible()
    }
    FilteredImageWarning {
        id: filter
        width: parent.width - 2
        contentVisibiliy: view.contentVisibility
        contentWarning: view.contentWarning
        images: [postExternal.thumbUrl]
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.openLink(postExternal.uri)
        enabled: filter.imageVisible()
    }
}
