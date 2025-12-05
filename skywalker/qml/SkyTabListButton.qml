import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

AccessibleTabButton {
    property listviewbasic list
    property string name: list.name

    signal closed

    id: button
    implicitWidth: tabRow.implicitWidth + rightPadding

    contentItem: Row {
        id: tabRow
        height: parent.height
        spacing: 0

        ListAvatar {
            id: avatar
            anchors.verticalCenter: parent.verticalCenter
            width: parent.height
            avatarUrl: list.avatarThumb
            visible: !list.isNull()
            onClicked: button.click()
        }

        AccessibleText {
            id: tabText
            width: Math.min(implicitWidth, 150)
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 5
            rightPadding: 5
            elide: Text.ElideRight
            font: button.font
            color: button.checked ? guiSettings.accentColor : guiSettings.textColor
            text: name
        }

        SvgButton {
            id: closeButton
            anchors.verticalCenter: parent.verticalCenter
            implicitWidth: 30
            width: 30
            height: width
            imageMargin: 8
            Material.background: "transparent"
            iconColor: guiSettings.textColor
            svg: SvgOutline.close
            accessibleName: qsTr(`close ${name}`)
            onClicked: button.closed()
        }
    }
}
