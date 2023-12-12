import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property bool restrictReply
    required property bool allowMentioned
    required property bool allowFollowing

    width: parent.width
    contentHeight: restrictionColumn.height
    title: qsTr("Who can reply?")
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

    Column {
        id: restrictionColumn

        CheckBox {
            checked: !restrictReply
            text: qsTr("Everyone")
            onCheckedChanged: {
                restrictReply = !checked

                if (checked) {
                    allowMentioned = false
                    allowFollowing = false
                }
            }
        }
        CheckBox {
            checked: restrictReply && !allowMentioned && !allowFollowing
            text: qsTr("Nobody")
            onCheckedChanged: {
                if (checked) {
                    restrictReply = true
                    allowMentioned = false
                    allowFollowing = false
                }
            }
        }
        CheckBox {
            checked: allowMentioned
            text: qsTr("Users mentioned in your post")
            onCheckedChanged: {
                allowMentioned = checked

                if (checked)
                    restrictReply = true
            }
        }
        CheckBox {
            checked: allowFollowing
            text: qsTr("Users you follow")
            onCheckStateChanged: {
                allowFollowing = checked

                if (checked)
                    restrictReply = true
            }
        }
    }
}
