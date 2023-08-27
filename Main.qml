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
            required property basicprofile author
            required property string postText
            required property int postCreatedSecondsAgo
            required property string postRepostedByName
            required property list<imageview> postImages
            required property var postExternal // externalview (var allows NULL)
            required property var postRecord // recordview

            id: postEntry
            columns: 2
            width: timelineView.width

            Rectangle {
                width: 25
                color: "transparent"
                visible: postRepostedByName
            }

            Text {
                width: parent.width - 25 - 10
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: "Reposted by " + postRepostedByName
                color: "darkslategrey"
                font.bold: true
                font.pointSize: 8
                visible: postRepostedByName
            }

            Avatar {
                id: avatar
                width: 25
                Layout.alignment: Qt.AlignTop
                avatarUrl: author.avatarUrl
            }

            Column {
                id: postColumn
                width: parent.width - avatar.width - 10

                PostHeader {
                    width: parent.width
                    authorName: author.name
                    postCreatedSecondsAgo: postEntry.postCreatedSecondsAgo
                }

                PostBody {
                    width: parent.width
                    postText: postEntry.postText
                    postImages: postEntry.postImages
                    postExternal: postEntry.postExternal
                    postRecord: postEntry.postRecord
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

    Component.onCompleted: {
        loginDialog.show()
    }
}
