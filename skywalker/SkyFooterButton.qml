import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Rectangle {
    property bool floating: false
    property SvgImage svg
    property int counter: 0
    property string counterBackgroundColor: guiSettings.badgeColor
    property string counterBorderColor: guiSettings.badgeBorderColor
    property string counterTextColor: guiSettings.badgeTextColor
    property bool showAltBadge
    property SvgImage altBadgeSvg

    signal clicked

    id: button
    color: "transparent"

    Accessible.role: Accessible.PageTab
    Accessible.onPressAction: clicked()

    RoundButton {
        width: height
        height: button.height
        radius: width / 2
        anchors.horizontalCenter: button.horizontalCenter
        Material.background: guiSettings.buttonNeutralColor
        display: AbstractButton.TextOnly
        visible: floating
    }

    SkySvg {
        id: icon
        y: height + 5
        width: height
        height: button.height - 10
        anchors.horizontalCenter: button.horizontalCenter
        color: guiSettings.textColor
        svg: button.svg

        BadgeCounter {
            color: button.counterBackgroundColor
            counterColor: button.counterTextColor
            counter: button.counter
        }

        // Alternative badge
        Loader {
            active: Boolean(altBadgeSvg)

            sourceComponent: Rectangle {
                x: icon.width - 17
                y: -icon.y + 6
                width: 20
                height: 20
                radius: 10
                color: floatingButtons ? guiSettings.buttonNeutralColor : guiSettings.backgroundColor
                border.color: guiSettings.backgroundColor
                border.width: 2
                visible: showAltBadge

                SkySvg {
                    x: 4
                    y: height + 2
                    width: 14
                    height: width
                    color: guiSettings.textColor
                    svg: altBadgeSvg
                }
            }
        }
    }

    MouseArea {
        anchors.fill: button
        onClicked: button.clicked()
    }
}
