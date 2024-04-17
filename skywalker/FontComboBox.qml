import QtQuick
import QtQuick.Controls

ComboBox {
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

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        fontComboBox.contentItem.color = guiSettings.buttonColor
        fontComboBox.indicator.color = guiSettings.buttonColor
    }
}

