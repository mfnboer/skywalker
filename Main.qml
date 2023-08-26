import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import skywalker

Window {
    width: 480
    height: 960
    visible: true
    title: qsTr("Skywalker")

    ListView {
        id: timelineView
        anchors.fill: parent
        spacing: 5
        model: skywalker.timelineModel

        delegate: GridLayout {
            required property string authorName
            required property string authorAvatar
            required property string postText
            required property int postCreatedSecondsAgo
            required property list<var> postImages // ImageView

            columns: 2
            width: timelineView.width

            Avatar {
                id: avatar
                width: 25
                Layout.alignment: Qt.AlignTop
                avatarUrl: authorAvatar
            }

            Column {
                id: postColumn
                width: parent.width - avatar.width - 10

                RowLayout {
                    width: parent.width

                    Text {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: authorName
                        font.bold: true
                    }
                    Text {
                        text: durationToString(postCreatedSecondsAgo)
                        font.pointSize: 8
                        color: "grey"
                    }
                }
                Text {
                    width: parent.width
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: postText
                    bottomPadding: postImages.length > 0 ? 5 : 0
                }

                Component.onCompleted: {
                    if (postImages.length > 0) {
                        let qmlFile = `ImagePreview${(postImages.length)}.qml`
                        let component = Qt.createComponent(qmlFile)
                        component.createObject(postColumn, {images: postImages})
                    }
                }
            }

            Rectangle {
                Layout.columnSpan: 2
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
            return Math.round(duration) + qsTr("d", "days")

        duration = duration / 30.4368499
        if (duration < 36)
            return Math.round(duration) + qsTr("mo", "months")

        duration = duration / 12
        return Math.round(duration) + qsTr("yr", "years")
    }

    Component.onCompleted: {
        loginDialog.show()
    }
}
