import QtQuick
import QtQuick.Controls

ComboBox {
    property bool pressedWithVirtualKeyboard: false

    id: fontComboBox
    height: 34
    model: ["Normal", "𝗕𝗼𝗹𝗱", "𝘐𝘵𝘢𝘭𝘪𝘤", "S̶t̶r̶i̶k̶e̶", "𝙼𝚘𝚗𝚘", "Sᴍᴀʟʟ ᴄᴀᴘs", "𝓒𝓾𝓻𝓼𝓲𝓿𝓮", "Ｗｉｄｅ", "Ⓑⓤⓑⓑⓛⓔ", "🅂🅀🅄🄰🅁🄴"]

    background: Rectangle {
        implicitWidth: 150
        implicitHeight: 34
        border.color: guiSettings.buttonColor
        border.width: 2
        color: "transparent"
    }

    onPressedChanged: {
        // On Android, a press on the combobox makes the virtual keyboard to close.
        // This causes to popup to close or not open at all. Open it after the
        // keyboard has closed.
        if (pressed && Qt.inputMethod.keyboardRectangle.y > 0)
            pressedWithVirtualKeyboard = true
    }

    Accessible.ignored: true

    GuiSettings {
        id: guiSettings
    }

    function virtualKeyboardClosed() {
        if (pressedWithVirtualKeyboard) {
            pressedWithVirtualKeyboard = false

            if (!popup.opened)
                popup.open()
        }
    }

    Component.onCompleted: {
        fontComboBox.contentItem.color = guiSettings.buttonColor
        fontComboBox.indicator.color = guiSettings.buttonColor
    }
}

