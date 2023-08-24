import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import skywalker

Window {
    width: 480
    height: 960
    visible: true
    title: qsTr("Skywalker")

    ListView {
        id: timelineView
        anchors.fill: parent
        model: skywalker.timelineModel
        delegate: ColumnLayout {
            required property string authorName
            required property string postText

            width: timelineView.width

            Label {
                width: parent.width
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                textFormat: Text.MarkdownText
                text: "**" + authorName + "**\n\n" + postText
            }
            Rectangle {
                width: parent.width
                color: "lightgrey"
                Layout.preferredHeight: 1
                Layout.fillWidth: true
            }
        }
    }

    Login {
        id: loginDialog
        anchors.centerIn: parent
        onAccepted: skywalker.login(user, password, host)

    }

    Skywalker {
        id: skywalker
        onLoginOk: skywalker.getTimeline()
        onLoginFailed: (error) => loginDialog.show(error)
    }

    Component.onCompleted: {
        loginDialog.show()
    }
}
