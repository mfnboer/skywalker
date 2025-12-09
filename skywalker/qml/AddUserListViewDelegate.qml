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
    Accessible.name: `${listTypeNameText.text} ${list.name}`

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

            AccessibleText {
                id: listTypeNameText
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.listTypeName(list.purpose)

                Accessible.ignored: true
            }
        }

        AccessibleCheckBox {
            width: undefined
            Layout.fillWidth: false
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignTop
            Accessible.name: memberCheck === QEnums.TRIPLE_BOOL_YES ? qsTr("in list") : qsTr("not in list")
            checked: memberCheck === QEnums.TRIPLE_BOOL_YES
            visible: memberCheck !== QEnums.TRIPLE_BOOL_UNKNOWN
            onCheckedChanged: updateList(checked)
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


    function updateList(add) {
        switch (memberCheck) {
        case QEnums.TRIPLE_BOOL_NO:
            if (add)
                addToList(list.uri)
            break
        case QEnums.TRIPLE_BOOL_YES:
            if (!add)
                removeFromList(list.uri, memberListItemUri)
            break
        }
    }
}
