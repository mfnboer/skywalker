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
            required property int createdSecondsAgo

            width: timelineView.width

            RowLayout {
                Text {
                    Layout.fillWidth: true
                    text: authorName
                    font.bold: true
                }
                Text {
                    text: durationToString(createdSecondsAgo)
                    font.pointSize: 8
                    color: "grey"
                }
            }
            Text {
                width: parent.width
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: postText
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

    function durationToString(duration) {
        if (duration < 60)
            return duration + qsTr("s", "seconds")

        duration = duration / 60
        if (duration < 60)
            return Math.round(duration) + qsTr("m", "minutes")

        duration = duration / 60
        if (duration < 24)
            return Math.round(duration) + qsTr("h", "hours")

        duration = duration / 24
        if (duration < 30.4368499)
            return Math.round(duration) + qStr("mo", "months")

        duration = duration / 30.4368499
        return Math.round(duration) + qStr("yr", "years")
    }

    Component.onCompleted: {
        loginDialog.show()
    }
}
