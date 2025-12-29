import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Dialog {
    property string userDid
    required property basicprofile author
    property var skywalker: root.getSkywalker(userDid)

    id: page
    width: parent.width - 40
    contentHeight: nameText.height + handleText.height + verifiedByText.height + verifierList.contentHeight + 20
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    SkyCleanedText {
        id: nameText
        x: 10
        width: parent.width - 20
        wrapMode: Text.Wrap
        font.bold: true
        font.pointSize: guiSettings.scaledFont(10/8)
        color: guiSettings.textColor
        plainText: qsTr(`${author.name} is verified`)
    }

    AccessibleText {
        id: handleText
        x: 10
        anchors.top: nameText.bottom
        width: parent.width - 20
        elide: Text.ElideRight
        color: guiSettings.handleColor
        text: `@${author.handle}`
    }

    AccessibleText {
        id: verifiedByText
        x: 10
        anchors.top: handleText.bottom
        anchors.topMargin: 10
        width: parent.width - 20
        text: qsTr("Verified by:")
    }

    SkyListView {
        id: verifierList
        anchors.topMargin: 10
        anchors.top: verifiedByText.bottom
        anchors.bottom: parent.bottom
        width: parent.width - 20
        model: author.verificationState.getValidVerifications()
        boundsBehavior: ListView.StopAtBounds
        clip: true
        spacing: 5

        delegate: Rectangle {
            required property verificationview modelData
            property basicprofile issuer

            width: verifierList.width
            height: row.height
            color: "transparent"

            RowLayout {
                id: row
                width: parent.width
                spacing: 10

                // Avatar
                Rectangle {
                    width: 60
                    Layout.fillHeight: true
                    color: "transparent"

                    Avatar {
                        x: 10
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 13
                        userDid: page.userDid
                        author: issuer
                        onClicked: {
                            skywalker.getDetailedProfile(modelData.issuer)
                            page.accept()
                        }
                    }
                }

                Column {
                    Layout.fillWidth: true
                    spacing: 1

                    AuthorNameAndStatus {
                        width: parent.width
                        userDid: page.userDid
                        author: issuer
                    }

                    AccessibleText {
                        width: parent.width
                        elide: Text.ElideRight
                        font.pointSize: guiSettings.scaledFont(7/8)
                        color: guiSettings.handleColor
                        text: `@${issuer.handle}`
                    }

                    AccessibleText {
                        width: parent.width
                        font.pointSize: guiSettings.scaledFont(7/8)
                        color: guiSettings.handleColor
                        text: modelData.createdAt.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                    }
                }
            }

            SkyMouseArea {
                anchors.fill: parent
                z: -1
                onClicked: {
                    skywalker.getDetailedProfile(modelData.issuer)
                    page.accept()
                }
            }

            ProfileUtils {
                id: profileUtils
                skywalker: page.skywalker

                onBasicProfileOk: (profile) => issuer = profile
            }

            Component.onCompleted: {
                profileUtils.getBasicProfile(modelData.issuer)
            }
        }
    }
}
