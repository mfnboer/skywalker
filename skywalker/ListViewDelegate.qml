import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property int viewWidth
    required property listview list
    required property profile listCreator
    property bool ownLists: true
    property int margin: 10
    property int maxTextLines: 1000

    id: view
    width: grid.width
    height: grid.height
    color: guiSettings.backgroundColor

    signal listClicked(listview list)
    signal updateList(listview list)
    signal deleteList(listview list)

    GridLayout {
        id: grid
        columns: 3
        width: viewWidth
        rowSpacing: 0

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 10
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

        Rectangle {
            width: 80
            Layout.fillHeight: true
            color: "transparent"

            SvgButton {
                id: membersButton
                anchors.right: moreButton.left
                width: 40
                height: width
                svg: svgOutline.group
                onClicked: root.viewListFeedDescription(list)
            }

            SvgButton {
                id: moreButton
                anchors.right: parent.right
                width: 40
                height: width
                svg: svgOutline.moreVert
                onClicked: moreMenu.open()

                Menu {
                    id: moreMenu

                    MenuItem {
                        text: qsTr("Edit")
                        enabled: ownLists
                        onTriggered: updateList(list)

                        MenuItemSvg {
                            svg: svgOutline.edit
                        }
                    }

                    MenuItem {
                        text: qsTr("Delete")
                        enabled: ownLists
                        onTriggered: deleteList(list)

                        MenuItemSvg {
                            svg: svgOutline.delete
                        }
                    }

                    MenuItem {
                        text: qsTr("Share")
                        onTriggered: skywalker.shareList(list)

                        MenuItemSvg {
                            svg: svgOutline.share
                        }
                    }
                }
            }
        }

        Text {
            topPadding: 5
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.leftMargin: view.margin
            Layout.rightMargin: view.margin
            wrapMode: Text.Wrap
            maximumLineCount: maxTextLines
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: list.formattedDescription
            visible: text

            onLinkActivated: (link) => root.openLink(link)
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 10
            color: "transparent"
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas on top
        anchors.fill: parent
        onClicked: {
            console.debug("LIST CLICKED:", list.name)
            view.listClicked(list)
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
