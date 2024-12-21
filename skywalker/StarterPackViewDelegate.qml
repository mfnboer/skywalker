import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    readonly property int margin: 10
    required property starterpackviewbasic starterPack
    property int maxTextLines: 25

    id: view
    height: viewColumn.height
    color: "transparent"

    Column {
        id: viewColumn
        x: margin
        width: parent.width - 2 * margin

        GridLayout {
            columns: 2
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

            SkyCleanedTextLine {
                topPadding: 10
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: starterPack.name
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
            contentLabels: starterPack.labels
            contentAuthorDid: starterPack.creator.did
        }

        SkyCleanedText {
            topPadding: 10
            width: parent.width
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            maximumLineCount: maxTextLines
            color: guiSettings.textColor
            plainText: starterPack.description
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

    MouseArea {
        z: -2 // Let other mouse areas on top
        anchors.fill: parent
        onClicked: skywalker.getStarterPackView(starterPack.uri)
    }

}
