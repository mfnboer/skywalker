import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

TabButton {
    signal closed

    id: button
    rightPadding: 0
    width: implicitWidth
    Accessible.name: qsTr(`Press to show ${text}`)

    contentItem: Row {
        Text {
            anchors.verticalCenter: parent.verticalCenter
            font: button.font
            color: button.checked ? guiSettings.accentColor : guiSettings.textColor
            text: button.text
        }

        SvgButton {
            anchors.verticalCenter: parent.verticalCenter
            width: 30
            height: width
            imageMargin: 8
            Material.background: "transparent"
            iconColor: guiSettings.textColor
            svg: SvgOutline.close
            accessibleName: qsTr(`close ${button.text}`)
            onClicked: button.closed()
        }
    }
}
