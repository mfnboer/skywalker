import QtQuick
import QtQuick.Controls

ComboBox {
    property int radius: 0
    property string borderColor: guiSettings.buttonColor
    property string color: guiSettings.buttonColor

    id: control
    valueRole: "value"
    textRole: "text"
    popup.width: width
    popup.topMargin: guiSettings.headerMargin
    popup.bottomMargin: guiSettings.footerMargin

    background: Rectangle {
        implicitWidth: 140
        implicitHeight: 34
        radius: control.radius
        border.color: control.borderColor
        border.width: 1
        color: "transparent"
    }

    indicator: Item {}

    contentItem: AccessibleText {
        leftPadding: 10
        rightPadding: 10
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        color: control.color
        text: control.displayText ? control.displayText : ""
    }

    delegate: ItemDelegate {
        required property int index
        required property var modelData

        id: delegate
        width: popup.width
        highlighted: control.highlightedIndex === index

        contentItem: AccessibleText {
            width: delegate.width
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: delegate.index === control.currentIndex ? guiSettings.buttonColor : guiSettings.textColor
            text: modelData.text
        }

        background: Rectangle {
            implicitWidth: delegate.width
            color: delegate.highlighted ? Material.listHighlightColor : "transparent"
        }
    }
}
