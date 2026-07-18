import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyDialog {
    property string userDid
    required property basicprofile author
    property list<verificationview> userVerifications: []
    property var skywalker: root.getSkywalker(userDid)

    id: page
    width: parent.width - 40
    contentHeight: nameText.height + handleText.height + verifiedByText.height + verifierList.contentHeight + viewText.height + 20
    standardButtons: Dialog.Ok
    anchors.centerIn: parent

    AccessibleText {
        id: nameText
        x: 10
        width: parent.width - 20
        wrapMode: Text.Wrap
        font.bold: true
        font.pointSize: guiSettings.scaledFont(10/8)
        text: qsTr(`${author.name} is verified`)
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
        anchors.bottom: viewText.top
        width: parent.width - 20
        model: page.getVerifications()
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
                        //anchors.verticalCenter: parent.verticalCenter
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
                        elide: Text.ElideRight
                        font.pointSize: guiSettings.scaledFont(7/8)
                        color: guiSettings.handleColor
                        text: modelData.createdAt.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                    }

                    AccessibleText {
                        width: parent.width
                        topPadding: 10
                        wrapMode: Text.Wrap
                        color: guiSettings.errorColor
                        text: "⚠️ " + getInvalidReason(modelData)
                        visible: !modelData.isValid
                    }

                    AccessibleText {
                        width: parent.width
                        wrapMode: Text.Wrap
                        textFormat: Text.StyledText
                        text: `<b>Verified name:</b><br>${modelData.verifiedDisplayName}`
                        visible: !modelData.isValid && modelData.verifiedDisplayName !== author.displayName
                    }

                    AccessibleText {
                        width: parent.width
                        wrapMode: Text.Wrap
                        textFormat: Text.StyledText
                        text: `<b>Verified handle:</b><br>${modelData.verifiedHandle}`
                        visible: !modelData.isValid && modelData.verifiedHandle !== author.handle
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

    AccessibleText {
        id: viewText
        x: 10
        width: parent.width - 20
        anchors.bottom: parent.bottom
        topPadding: 10
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        textFormat: Text.RichText
        text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">View trusted verifiers</a>`)
        onLinkActivated: {
            root.viewTrustedVerifiers()
            page.accept()
        }
    }

    Connections {
        target: skywalker.getVerificationUtils()

        function onVerifications(did, verifications) {
            if (did !== author.did)
                return

            userVerifications = verifications
        }
    }

    function getInvalidReason(verification) {
        if (verification.verifiedDisplayName && verification.verifiedDisplayName !== author.displayName) {
            if (verification.verifiedHandle && verification.verifiedHandle !== author.handle)
                return qsTr("Name and handle changed after verification")
            else
                return qsTr("Name changed after verification")
        }

        if (verification.verifiedHandle && verification.verifiedHandle !== author.handle)
            return qsTr("Handle changed after verification")

        return "unknown"
    }

    function getVerifications() {
        return author.verificationState.getValidVerifications().concat(userVerifications)
    }

    Component.onCompleted: {
        skywalker.getVerificationUtils().getVerifications(author)
    }
}
