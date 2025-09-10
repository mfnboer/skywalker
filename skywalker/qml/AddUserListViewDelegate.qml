import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property listview list
    required property profile listCreator
    required property int memberCheck // QEnums.TripleBool
    required property string memberListItemUri
    property int margin: 10

    signal addToList(string listUri)
    signal removeFromList(string listUri, string listItemUri)

    id: view
    height: grid.height
    color: guiSettings.backgroundColor

    Accessible.role: Accessible.StaticText
    Accessible.name: `${listTypeNameText.text} ${list.name} by ${listCreatorHandleText.text}`

    GridLayout {
        id: grid
        columns: 3
        width: parent.width
        rowSpacing: 0

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 10
            color: "transparent"
        }

        ListAvatar {
            Layout.leftMargin: view.margin
            Layout.rightMargin: view.margin
            x: 8
            y: 5
            width: 44
            Layout.alignment: Qt.AlignTop
            avatarUrl: list.avatarThumb
        }

        Column {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: view.margin

            SkyCleanedTextLine {
                id: listNameText
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: list.name

                Accessible.ignored: true
            }

            Text {
                id: listTypeNameText
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.listTypeName(list.purpose)

                Accessible.ignored: true
            }

            Text {
                id: listCreatorHandleText
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${listCreator.handle}`

                Accessible.ignored: true
            }
        }

        SvgButton {
            Layout.alignment: Qt.AlignTop
            svg: memberCheck === QEnums.TRIPLE_BOOL_YES ? SvgOutline.remove : SvgOutline.add
            accessibleName: memberCheck === QEnums.TRIPLE_BOOL_YES ? qsTr("add") : qsTr("remove")
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
