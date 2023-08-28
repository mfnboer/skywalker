import QtQuick
import QtQuick.Layouts
import skywalker

Item {
    property externalview postExternal

    width: parent.width
    height: externalColumn.height

    RoundedFrame {
        id: externalContent
        width: parent.width
        objectToRound: externalColumn

        Column {
            id: externalColumn
            width: parent.width
            spacing: 5
            visible: false

            Image {
                id: thumbImg
                width: parent.width
                source: postExternal.thumbUrl
                fillMode: Image.PreserveAspectFit
                visible: postExternal.thumbUrl
            }
            Text {
                id: linkText
                width: parent.width - 10
                leftPadding: 5
                rightPadding: 5
                text: new URL(postExternal.uri).hostname
                elide: Text.ElideRight
                font.pointSize: 8
                color: "grey"
            }
            Text {
                id: titleText
                width: parent.width - 10
                leftPadding: 5
                rightPadding: 5
                text: postExternal.title
                wrapMode: Text.Wrap
                font.bold: true
            }
            Text {
                id: descriptionText
                width: parent.width - 10
                leftPadding: 5
                rightPadding: 5
                bottomPadding: 5
                text: postExternal.description
                wrapMode: Text.Wrap
            }
        }
    }
    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: "lightgrey"
        color: "transparent"
        radius: externalContent.radius

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { Qt.openUrlExternally(postExternal.uri); }
        }
    }
}
