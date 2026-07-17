import QtQuick
import QtQuick.Controls

ComboBox {
    property int radius: 5
    property string borderColor: guiSettings.buttonColor
    property string textColor: guiSettings.textColor

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
        border.width: control.activeFocus ? 1 : 0
        color: guiSettings.textInputBackgroundColor
    }

    contentItem: AccessibleText {
        leftPadding: 10
        rightPadding: 10
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.pointSize: guiSettings.scaledFont(9/8)
        color: control.textColor
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

    Component.onCompleted: {
        indicator.color = control.textColor
    }
}
