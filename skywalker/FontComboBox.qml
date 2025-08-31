import QtQuick
import QtQuick.Controls

ComboBox {
    id: fontComboBox
    height: 22
    model: [
        { display: "Aa", font: "Normal" },
        { display: "𝗔𝗮", font: "𝗕𝗼𝗹𝗱" },
        { display: "𝘈𝘢", font: "𝘐𝘵𝘢𝘭𝘪𝘤" },
        { display: "A̶𝘢̶", font: "S̶t̶r̶i̶k̶e̶" },
        { display: "𝙰𝚊", font: "𝙼𝚘𝚗𝚘" },
        { display: "Aᴀ", font: "Sᴍᴀʟʟ ᴄᴀᴘs" },
        { display: "𝓐𝓪", font: "𝓒𝓾𝓻𝓼𝓲𝓿𝓮" },
        { display: "Ａａ", font: "Ｗｉｄｅ" },
        { display: "Ⓐⓐ", font: "Ⓑⓤⓑⓑⓛⓔ" },
        { display: "🄰🄰", font: "🅂🅀🅄🄰🅁🄴" }
    ]
    valueRole: "display"
    textRole: "display"
    popup.width: 130
    popup.topMargin: guiSettings.headerMargin
    popup.bottomMargin: guiSettings.footerMargin

    background: Rectangle {
        radius: 3
        implicitWidth: 20
        border.color: guiSettings.buttonColor
        border.width: 1
        color: "transparent"
    }

    indicator: Item {}

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        text: fontComboBox.displayText
    }

    delegate: ItemDelegate {
        required property int index
        required property var modelData

        id: delegate
        width: popup.width

        contentItem: Text {
            width: delegate.width
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: delegate.index === fontComboBox.currentIndex ? guiSettings.buttonColor : guiSettings.textColor
            text: delegate.modelData.font
        }
    }

    Component.onCompleted: {
        fontComboBox.contentItem.color = guiSettings.buttonColor
    }
}

