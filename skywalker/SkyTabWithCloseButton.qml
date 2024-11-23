import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

TabButton {
    property color backgroundColor: "transparent"
    property basicprofile profile
    property bool showDot: false

    signal closed

    id: button
    rightPadding: 0
    width: implicitWidth
    Accessible.name: qsTr(`Press to show ${text}`)

    contentItem: Row {
        id: tabRow

        Avatar {
            anchors.verticalCenter: parent.verticalCenter
            width: 24
            author: profile
            visible: !profile.isNull()
        }

        Rectangle {
            width: 5
            height: 30
            color: "transparent"
            visible: !profile.isNull()
        }

        Text {
            width: Math.min(implicitWidth, 150)
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
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

    SkyDot {
        anchors.rightMargin: 22
        visible: showDot
    }

    Rectangle {
        anchors.fill: parent
        z: contentItem.z - 1
        color: parent.backgroundColor
        opacity: guiSettings.focusHighlightOpacity
    }
}
