import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Dialog {
    required property activitysubscription subscription
    property bool post: subscription.post
    property bool reply: subscription.reply

    signal subscribeTo(bool post, bool reply)

    width: parent.width
    contentHeight: activityCol.height
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    onAccepted: {
        if (post === subscription.post && reply === subscription.reply)
            reject()
        else
            subscribeTo(post, reply)
    }

    Column {
        id: activityCol
        width: parent.width

        AccessibleText {
            width: parent.width
            topPadding: 10
            bottomPadding: 10
            font.bold: true
            text: qsTr("Get notified of this account's activity")
        }

        AccessibleCheckBox {
            text: qsTr("Posts")
            checked: post
            onCheckedChanged: {
                post = checked

                if (!post)
                    reply = false
            }
        }

        AccessibleCheckBox {
            text: qsTr("Replies")
            checked: reply
            onCheckedChanged: {
                reply = checked

                if (reply)
                    post = true
            }
        }
    }
}
