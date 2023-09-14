import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    property externalview postExternal

    width: parent.width
    height: externalColumn.height

    RoundedFrame {
        id: externalContent
        objectToRound: externalColumn
        anchors.fill: parent
        border.width: 1
        border.color: "lightgrey"

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
                color: "blue"
            }
            Text {
                id: titleText
                width: parent.width - 10
                leftPadding: 5
                rightPadding: 5
                color: Material.foreground
                text: postExternal.title
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
                font.bold: true
                visible: postExternal.title
            }
            Text {
                id: descriptionText
                width: parent.width - 10
                leftPadding: 5
                rightPadding: 5
                bottomPadding: 5
                color: Material.foreground
                text: postExternal.description ? postExternal.description : postExternal.uri
                wrapMode: Text.Wrap
                maximumLineCount: 5
                elide: Text.ElideRight
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: Qt.openUrlExternally(postExternal.uri)
    }
}
