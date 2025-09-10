import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Dialog {
    required property string txt
    readonly property list<string> emojiList: UnicodeFonts.getUniqueEmojis(txt)

    id: page
    width: parent.width - 40
    contentHeight: emojiListView.contentHeight
    modal: true
    title: qsTr("Emoji names")
    standardButtons: Dialog.Ok
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    SkyListView {
        id: emojiListView
        width: parent.width
        height: parent.height
        model: emojiList
        clip: true
        spacing: 5

        delegate: RowLayout {
            required property string modelData

            width: emojiListView.width
            spacing: 10

            AccessibleText {
                text: modelData
                font.pointSize: guiSettings.scaledFont(2)
                font.family: UnicodeFonts.getEmojiFontFamily()
            }

            AccessibleText {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: emojiNames.getEmojiName(modelData)
            }
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y: emojiListView.verticalOvershoot < 0 ? -emojiListView.verticalOvershoot - height : emojiListView.height - emojiListView.verticalOvershoot
            width: 150
            fillMode: Image.PreserveAspectFit
            source: "/images/dont_panic.png"
            visible: emojiListView.verticalOvershoot !== 0
        }
    }

    EmojiNames {
        id: emojiNames
    }
}
