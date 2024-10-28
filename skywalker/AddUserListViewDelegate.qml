import QtQuick
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
            Layout.preferredHeight: 10
            color: "transparent"
        }

        ListAvatar {
            Layout.leftMargin: view.margin + 8
            Layout.rightMargin: view.margin + 5
            Layout.preferredWidth: 44
            Layout.alignment: Qt.AlignTop
            avatarUrl: view.list.avatarThumb
        }

        Column {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: view.margin

            SkyCleanedText {
                id: listNameText
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: view.list.name

                Accessible.ignored: true
            }

            Text {
                id: listTypeNameText
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.listTypeName(view.list.purpose)

                Accessible.ignored: true
            }

            Text {
                id: listCreatorHandleText
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${view.listCreator.handle}`

                Accessible.ignored: true
            }
        }

        SvgButton {
            Layout.alignment: Qt.AlignTop
            svg: view.memberCheck === QEnums.TRIPLE_BOOL_YES ? SvgOutline.remove : SvgOutline.add
            accessibleName: view.memberCheck === QEnums.TRIPLE_BOOL_YES ? qsTr("add") : qsTr("remove")
            visible: view.memberCheck !== QEnums.TRIPLE_BOOL_UNKNOWN
            onClicked: view.updateList()
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
