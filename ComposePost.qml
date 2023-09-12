import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    property int maxPostLength: 300
    signal closed

    id: page
    topPadding: 10
    bottomPadding: 10

    header: Rectangle {
        width: parent.width
        height: root.headerHeight
        z: 10
        color: "black"

        RoundButton {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            Material.background: "blue"
            contentItem: Text {
                color: "white"
                text: qsTr("Cancel")
            }

            onClicked: page.closed()
        }

        Button {
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            Material.background: "blue"

            contentItem: Text {
                color: "white"
                text: qsTr("Post", "verb on post button")
            }

            enabled: postText.text.length > 0 && postText.text.length <= maxPostLength
            onClicked: page.closed()
        }
    }

    footer: Rectangle {
        width: parent.width
        height: root.footerHeight
        z: 10
        color: "white"

        ProgressBar {
            id: textLengthBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            from: 0
            to: page.maxPostLength
            value: postText.text.length

            contentItem: Rectangle {
                width: textLengthBar.visualPosition * parent.width
                height: parent.height
                color: textLengthBar.value <= textLengthBar.to ? "blue" : "red"
            }
        }

        Text {
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: maxPostLength - postText.text.length
        }
    }

    TextArea {
        id: postText
        anchors.fill: parent
        placeholderText: text.length === 0 ? "Say something nice" : ""
        placeholderTextColor: "grey"
        wrapMode: TextEdit.Wrap
        focus: true
        background: Rectangle {
            border.color: "transparent"
        }
    }
}
