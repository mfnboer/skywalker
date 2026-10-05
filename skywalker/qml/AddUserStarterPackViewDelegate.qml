import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property starterpackview starterPack
    required property int memberCountDelta
    required property int memberCheck // QEnums.TripleBool
    required property string memberListItemUri
    property int margin: 10

    signal addToStarterPack(starterpackview starterPack)
    signal removeFromStarterPack(starterpackview starterPack, string listItemUri)

    id: view
    height: grid.height
    color: guiSettings.backgroundColor

    Accessible.role: Accessible.StaticText
    Accessible.name: starterPack.name

    GridLayout {
        id: grid
        columns: 3
        width: parent.width
        rowSpacing: 0

        Rectangle {
            Layout.leftMargin: view.margin
            Layout.rightMargin: view.margin
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            Layout.alignment: Qt.AlignTop
            color: "transparent"

            SkySvg {
                // y: height + 10
                width: parent.width
                height: width
                color: guiSettings.starterpackColor
                svg: SvgOutline.starterpack
            }
        }

        Column {
            spacing: 0
            Layout.fillWidth: true
            Layout.rightMargin: view.margin
            Layout.alignment: Qt.AlignVCenter

            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                text: starterPack.name

                Accessible.ignored: true
            }

            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.getMemberCountString(starterPack.listItemCount + memberCountDelta)
                visible: starterPack.listItemCount >= 0
            }
        }

        AccessibleCheckBox {
            width: undefined
            Layout.fillWidth: false
            Layout.preferredWidth: 38
            Layout.alignment: Qt.AlignVCenter
            Accessible.name: memberCheck === QEnums.TRIPLE_BOOL_YES ? qsTr("in starter pack") : qsTr("not in starter pack")
            checked: memberCheck === QEnums.TRIPLE_BOOL_YES
            enabled: memberCheck === QEnums.TRIPLE_BOOL_YES || starterPack.listItemCount < starterPack.MAX_MEMBERS
            visible: memberCheck !== QEnums.TRIPLE_BOOL_UNKNOWN
            onCheckedChanged: updateStarterPack(checked)
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }
    }


    function updateStarterPack(add) {
        switch (memberCheck) {
        case QEnums.TRIPLE_BOOL_NO:
            if (add)
                addToStarterPack(starterPack)
            break
        case QEnums.TRIPLE_BOOL_YES:
            if (!add)
                removeFromStarterPack(starterPack, memberListItemUri)
            break
        }
    }
}
