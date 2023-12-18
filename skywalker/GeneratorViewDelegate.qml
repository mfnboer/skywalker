import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 10
    required property int viewWidth
    required property generatorview feed
    required property profile feedCreator
    required property bool endOfFeed
    property int maxTextLines: 1000

    signal feedClicked(generatorview feed)

    id: generatorView
    width: grid.width
    height: grid.height
    color: "transparent"

    GridLayout {
        id: grid
        rowSpacing: 0
        columns: 2
        width: viewWidth

        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            height: 5
            color: "transparent"
        }

        FeedAvatar {
            Layout.rowSpan: 3
            Layout.leftMargin: generatorView.margin
            Layout.rightMargin: generatorView.margin
            x: 8
            y: 5
            width: guiSettings.threadBarWidth * 5
            avatarUrl: feed.avatar

            onClicked: feedClicked(feed)
        }

        Text {
            Layout.fillWidth: true
            Layout.rightMargin: generatorView.margin
            elide: Text.ElideRight
            font.bold: true
            color: guiSettings.textColor
            text: feed.displayName
        }

        Text {
            Layout.fillWidth: true
            Layout.rightMargin: generatorView.margin
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: feedCreator.name
        }

        Text {
            Layout.fillWidth: true
            Layout.rightMargin: generatorView.margin
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: "@" + feedCreator.handle
        }

        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.leftMargin: generatorView.margin
            Layout.rightMargin: generatorView.margin
            wrapMode: Text.Wrap
            maximumLineCount: maxTextLines
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: feed.description
        }

        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            height: 5
            color: "transparent"
        }

        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }

        // End of feed indication
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            topPadding: generatorView.margin
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("End of feed")
            font.italic: true
            visible: endOfFeed
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas on top
        anchors.fill: parent
        onClicked: {
            console.debug("FEED CLICKED:", feed.displayName)
            generatorView.feedClicked(feed)
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
