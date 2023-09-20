import QtQuick
import QtQuick.Controls
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
                width: 24
                Layout.alignment: Qt.AlignTop
                avatarUrl: record.author.avatarUrl
            }

            PostHeader {
                Layout.fillWidth: true
                authorName: record.author.name
                authorHandle: record.author.handle
                postThreadType: QEnums.THREAD_NONE
                postIndexedSecondsAgo: (new Date() - record.postDateTime) / 1000
            }
        }

        PostBody {
            width: parent.width
            postText: record.postText
            postImages: record.images
            postExternal: record.external
            postDateTime: record.postDateTime
            maxTextLines: 6
            visible: record.available
        }

        Text {
            width: parent.width
            color: Material.foreground
            text: qsTr("NOT FOUND")
            visible: record.notFound
        }
        Text {
            width: parent.width
            color: Material.foreground
            text: qsTr("BLOCKED")
            visible: record.blocked
        }
        Text {
            width: parent.width
            color: Material.foreground
            text: qsTr("NOT SUPPORTED")
            visible: record.notSupported
        }
        Text {
            width: parent.width
            color: Material.color(Material.Grey)
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            text: record.unsupportedType
            visible: record.notSupported
        }
    }
    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: "lightgrey"
        color: "transparent"
        radius: 10
    }
    MouseArea {
        z: -1 // Let other mouse areas, e.g. images, get on top
        anchors.fill: parent
        onClicked: {
            console.debug("RECORD VIEW CLICKED:", record.postUri)
            if (record.postUri)
                skywalker.getPostThread(record.postUri)
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
