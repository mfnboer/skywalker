import QtQuick
import QtQuick.Controls

Rectangle {
    signal moreEmoji

    width: 50
    radius: width / 2
    height: buttonColumn.height + 10
    color: guiSettings.backgroundColor

    Column {
        anchors.centerIn: parent
        id: buttonColumn

        ReactionButton {
            text: "🙂"
        }
        ReactionButton {
            text: "😂"
        }
        ReactionButton {
            text: "❤️"
        }
        ReactionButton {
            text: "👍"
        }
        ReactionButton {
            text: "👎"
        }
        SvgButton {
            width: 40
            height: 40
            svg: SvgOutline.moreVert
            iconColor: guiSettings.textColor
            Material.background: guiSettings.postHighLightColor
            accessibleName: "more emoji"
            flat: true

            onClicked: moreEmoji()
        }
    }
}
