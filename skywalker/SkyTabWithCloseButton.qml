import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

AccessibleTabButton {
    property color backgroundColor: "transparent"
    property basicprofile profile
    property bool showCloseButton: true
    property bool showDot: false

    signal closed

    id: button
    rightPadding: 0

    // For some reason setting the width of closeButton to 0 when it is invisible
    // causes the tabbar not to scroll all the way to the left. It is not the
    // button itself. When I remove the button, the problem remains.
    width: avatar.width + whitespace.width + tabText.width + closeButton.width + leftPadding

    contentItem: Row {
        id: tabRow

        Avatar {
            id: avatar
            anchors.verticalCenter: parent.verticalCenter
            width: visible ? 34 : 0
            author: profile
            visible: !profile.isNull()
        }

        Rectangle {
            id: whitespace
            width: 5
            height: visible ? 30 : 0
            color: "transparent"
            visible: !profile.isNull()
        }

        Text {
            id: tabText
            width: Math.min(implicitWidth, 150)
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
            font: button.font
            color: button.checked ? guiSettings.accentColor : guiSettings.textColor
            text: button.text
        }

        SvgButton {
            id: closeButton
            anchors.verticalCenter: parent.verticalCenter
            width: 30
            height: width
            imageMargin: 8
            Material.background: "transparent"
            iconColor: guiSettings.textColor
            svg: SvgOutline.close
            accessibleName: qsTr(`close ${button.text}`)
            visible: showCloseButton
            onClicked: button.closed()
        }
    }

    SkyDot {
        anchors.rightMargin: 22
        visible: showDot
    }
}
