import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    readonly property int margin: 10
    property string userDid
    required property starterpackviewbasic starterPack
    property bool allowEdit: true
    property int maxTextLines: 25

    signal updateStarterPack(starterpackviewbasic starterPack)
    signal deleteStarterPack(starterpackviewbasic starterPack)

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
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: starterPack.name
            }

            Rectangle {
                Layout.preferredWidth: 40
                Layout.fillHeight: true
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

            AccessibleText {
                Layout.fillWidth: true
                Layout.fillHeight: true
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: qsTr(`by @${starterPack.creator.handle}`)
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
