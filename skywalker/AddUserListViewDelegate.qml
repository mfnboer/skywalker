import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property int viewWidth
    required property listview list
    required property profile listCreator
    required property int memberCheck // QEnums.TripleBool
    required property string memberListItemUri
    property int margin: 10

    signal addToList(string listUri)
    signal removeFromList(string listUri, string listItemUri)

    id: view
    width: grid.width
    height: grid.height
    color: guiSettings.backgroundColor

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
            width: 44
            Layout.fillHeight: true
            avatarUrl: list.avatar
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
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.listTypeName(list.purpose)
            }

            Text {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${listCreator.handle}`
            }
        }

        SvgButton {
            Layout.alignment: Qt.AlignTop
            svg: memberCheck === QEnums.TRIPLE_BOOL_YES ? svgOutline.remove : svgOutline.add
            visible: memberCheck !== QEnums.TRIPLE_BOOL_UNKNOWN

            onClicked: updateList()
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

    GuiSettings {
        id: guiSettings
    }

    function updateList() {
        switch (memberCheck) {
        case QEnums.TRIPLE_BOOL_NO:
            addToList(list.uri)
            break
        case QEnums.TRIPLE_BOOL_YES:
            removeFromList(list.uri, memberListItemUri)
            break
        }
    }
}
