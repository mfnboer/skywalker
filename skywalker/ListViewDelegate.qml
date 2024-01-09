import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property int viewWidth
    required property listview list
    required property profile listCreator
    property int margin: 10
    property int maxTextLines: 1000

    id: view
    width: grid.width
    height: grid.height
    color: guiSettings.backgroundColor

    signal listClicked(listview list)

    GridLayout {
        id: grid
        columns: 2
        width: viewWidth
        rowSpacing: 0

        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            height: 5
            color: "transparent"
        }

        FeedAvatar {
            Layout.leftMargin: view.margin
            Layout.rightMargin: view.margin
            x: 8
            y: 5
            width: guiSettings.threadBarWidth * 5
            avatarUrl: list.avatar

            onClicked: listClicked(list)
        }

        Column {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: view.margin

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: list.name
            }

            Text {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: listCreator.name


                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(listCreator.did)
                }
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + listCreator.handle


                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(listCreator.did)
                }
            }
        }

        Text {
            topPadding: 5
            bottomPadding: 10
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.leftMargin: view.margin
            Layout.rightMargin: view.margin
            wrapMode: Text.Wrap
            maximumLineCount: maxTextLines
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: list.formattedDescription

            onLinkActivated: (link) => root.openLink(link)
        }

        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }

    }

    GuiSettings {
        id: guiSettings
    }
}
