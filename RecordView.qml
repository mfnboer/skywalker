import QtQuick
import QtQuick.Layouts
import skywalker

Item {
    property recordview record

    width: parent.width
    height: recordColumn.height + 10

    Column {
        id: recordColumn
        width: parent.width - 10
        anchors.centerIn: parent

        RowLayout {
            width: parent.width
            visible: record.available

            Avatar {
                id: avatar
                width: 15
                Layout.alignment: Qt.AlignTop
                avatarUrl: record.author.avatarUrl
            }

            PostHeader {
                Layout.fillWidth: true
                authorName: record.author.name
                postIndexedSecondsAgo: record.createdSecondsAgo
            }
        }

        PostBody {
            width: parent.width
            postText: record.postText
            postImages: record.images
            postExternal: record.external
            maxTextLines: 6
            visible: record.available
        }

        Text {
            width: parent.width
            text: qsTr("NOT FOUND")
            visible: record.notFound
        }
        Text {
            width: parent.width
            text: qsTr("BLOCKED")
            visible: record.blocked
        }
    }
    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: "lightgrey"
        color: "transparent"
        radius: 10
    }
}
