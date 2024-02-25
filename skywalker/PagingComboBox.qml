import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ComboBox {
    required property bool inProgress
    property var bottomOvershootFun: null
    property int initialIndex: 0
    property string findValue
    property color backgroundColor: Material.dialogColor

    id: control

    signal valueFound

    popup: Popup {
        y: control.height - 1
        width: control.width
        implicitHeight: contentItem.implicitHeight
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            color: Material.dialogColor
            radius: 2
        }
    }

    background: Rectangle {
        implicitWidth: 140
        implicitHeight: 40
        color: backgroundColor
        border.color: contentItem.color
        border.width: 2
        radius: 2
    }

    onCountChanged: {
        if (findValue && currentIndex < 0) {
            const valueIndex = indexOfValue(findValue)

            if (valueIndex >= 0) {
                currentIndex = valueIndex
                valueFound()
            }
            else {
                control.bottomOvershootFun()
            }
        }

        if (count > 0 && currentIndex < 0)
            currentIndex = initialIndex
    }

    FlickableRefresher {
        inProgress: control.inProgress
        verticalOvershoot: popup.contentItem.verticalOvershoot
        bottomOvershootFun: control.bottomOvershootFun
        topText: ""
    }

    GuiSettings {
        id: guiSettings
    }
}
