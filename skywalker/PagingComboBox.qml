import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ComboBox {
    required property bool inProgress
    property var bottomOvershootFun: null
    property int initialIndex: 0

    id: control

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

    onCountChanged: {
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
