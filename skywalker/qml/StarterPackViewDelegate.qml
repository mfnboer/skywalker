import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    readonly property int margin: 10
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property starterpackview starterPack
    required property bool endOfFeed
    property bool allowEdit: true
    property int maxTextLines: 25

    signal updateStarterPack(starterpackview starterPack)
    signal deleteStarterPack(starterpackview starterPack)

    id: view
    height: viewColumn.height
    color: "transparent"

    Column {
        id: viewColumn
        x: margin
        width: parent.width - 2 * margin

        GridLayout {
            columns: 3
            rowSpacing: 0
            width: parent.width

            Rectangle {
                Layout.rowSpan: 2
                Layout.preferredWidth: 34
                Layout.preferredHeight: 44
                Layout.fillHeight: true
                color: "transparent"

                SkySvg {
                    y: height + 10
                    width: parent.width
                    height: width
                    color: guiSettings.starterpackColor
                    svg: SvgOutline.starterpack
                }
            }

            AccessibleText {
                topPadding: 10
                bottomPadding: 10
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: starterPack.name
            }

            Rectangle {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                Layout.topMargin: 8
                color: "transparent"

                SvgButton {
                    id: moreButton
                    anchors.right: parent.right
                    width: 40
                    height: width
                    svg: SvgOutline.moreVert
                    accessibleName: qsTr("more options")

                    onClicked: moreMenu.popup(moreButton)
                }
            }

            Column {
                spacing: 0
                Layout.fillWidth: true

                AuthorNameAndStatus {
                    width: parent.width
                    userDid: view.userDid
                    author: starterPack.creator

                    Accessible.role: Accessible.Link
                    Accessible.name: starterPack.creator.name
                    Accessible.onPressAction: skywalker.getDetailedProfile(starterPack.creator.did)

                    SkyMouseArea {
                        anchors.fill:  parent
                        onClicked: skywalker.getDetailedProfile(starterPack.creator.did)
                    }
                }

                AccessibleText {
                    width: parent.width
                    elide: Text.ElideRight
                    font.pointSize: guiSettings.scaledFont(7/8)
                    color: guiSettings.handleColor
                    text: `@${starterPack.creator.handle}`

                    Accessible.role: Accessible.Link
                    Accessible.name: text
                    Accessible.onPressAction: skywalker.getDetailedProfile(starterPack.creator.did)

                    SkyMouseArea {
                        anchors.fill:  parent
                        onClicked: skywalker.getDetailedProfile(starterPack.creator.did)
                    }
                }
            }
        }

        ContentLabels {
            id: contentLabels
            anchors.left: parent.left
            anchors.leftMargin: margin
            anchors.right: undefined
            userDid: view.userDid
            contentLabels: starterPack.labels
            contentAuthor: starterPack.creator
        }

        AccessibleText {
            topPadding: 10
            width: parent.width
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            maximumLineCount: maxTextLines
            color: guiSettings.textColor
            text: starterPack.description
            visible: starterPack.description
        }

        AccessibleText {
            topPadding: 10
            width: parent.width
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: guiSettings.getMemberCountString(starterPack.listItemCount)
            visible: starterPack.listItemCount >= 0
        }

        Rectangle {
            width: parent.width
            height: 10
            color: "transparent"
        }

        Rectangle {
            width: parent.width
            height: 1
            color: guiSettings.separatorColor
        }

        // End of feed indication
        AccessibleText {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            topPadding: view.margin
            bottomPadding: 50
            elide: Text.ElideRight
            text: qsTr("End of feed")
            font.italic: true
            visible: endOfFeed
        }
    }

    SkyMouseArea {
        z: -2 // Let other mouse areas on top
        anchors.fill: parent
        onClicked: root.getSkywalker(userDid).getStarterPackView(starterPack.uri)
    }

    SkyMenu {
        id: moreMenu

        SkyMenuButton {
            text: qsTr("Edit")
            svg: SvgOutline.edit
            popup: moreMenu
            visible: allowEdit && isOwnStarterPack()
            onClicked: updateStarterPack(starterPack)
        }

        SkyMenuButton {
            text: qsTr("Delete")
            svg: SvgOutline.delete
            popup: moreMenu
            visible: allowEdit && isOwnStarterPack()
            onClicked: deleteStarterPack(starterPack)
        }

        TranslateMenuButton {
            popup: moreMenu
            enabled: starterPack.description
            onClicked: root.translateText(starterPack.description)
        }
        SkyMenuButton {
            text: qsTr("Share")
            svg: SvgOutline.share
            popup: moreMenu
            onClicked: skywalker.getShareUtils().shareStarterPack(starterPack)
        }
        SkyMenuButton {
            text: qsTr("Copy starter pack link")
            svg: SvgOutline.link
            popup: moreMenu
            onClicked: skywalker.getShareUtils().copyUriToClipboard(starterPack.uri)
        }
        SkyMenuButton {
            text: qsTr("Report starter pack")
            svg: SvgOutline.report
            popup: moreMenu
            onClicked: root.reportStarterPack(starterPack, userDid)
        }
        SkyMenuButton {
            text: qsTr("Emoji names")
            svg: SvgOutline.emojiLanguage
            popup: moreMenu
            visible: UnicodeFonts.hasEmoji(starterPack.description)
            onClicked: root.showEmojiNamesList(starterPack.description)
        }
    }

    function isOwnStarterPack() {
        return skywalker.getUserDid() === starterPack.creator.did
    }
}
