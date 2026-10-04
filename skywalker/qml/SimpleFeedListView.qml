import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    required model
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)

    signal feedClicked(generatorview feed)

    id: feedListView
    Layout.preferredWidth: parent.width
    Layout.preferredHeight: parent.height
    spacing: 3

    clip: true

    Accessible.role: Accessible.List

    delegate: Rectangle {
        required property int index
        required property generatorview modelData
        property alias feed: feedEntry.modelData

        id: feedEntry
        width: feedListView.width
        height: grid.height
        color: guiSettings.backgroundColor

        Accessible.role: Accessible.Button
        Accessible.name: feed.name
        Accessible.onPressAction: feedClicked(feed)

        GridLayout {
            id: grid
            rowSpacing: 0
            columns: 2
            width: parent.width

            FeedAvatar {
                Layout.rowSpan: 3
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 50
                Layout.preferredHeight: 50
                userDid: feedListView.userDid
                avatarUrl: feed.avatarThumb
                contentMode: feed.contentMode
                unknownSvg: guiSettings.feedDefaultAvatar(feed)

                onClicked: feedClicked(feed)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr(`select feed: ${feed.displayName}`)
                Accessible.onPressAction: clicked()
            }

            AccessibleText {
                Layout.fillWidth: true
                width: parent.width
                bottomPadding: 5
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: feed.displayName
            }

            AuthorNameAndStatus {
                width: parent.width
                userDid: feedListView.userDid
                author: feed.creator
            }

            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + feed.creator.handle
            }

            Rectangle {
                Layout.topMargin: 3
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: feedEntry.index < feedListView.count - 1
            }
        }

        SkyMouseArea {
            z: -1
            anchors.fill: parent
            onClicked: feedClicked(feed)
        }
    }
}

