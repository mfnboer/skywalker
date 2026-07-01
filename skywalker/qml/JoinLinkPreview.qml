import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    property string userDid
    property string uri
    property string title
    property string code: uri ? skywalker.chat.getJoinLinkCodeFromUri(uri) : ""
    property joinlinkpreview joinLink
    readonly property basicprofile owner: joinLink.owner.basicProfile
    property Skywalker skywalker: root.getSkywalker()
    property string borderColor: guiSettings.isLightMode ? Qt.darker(color, 1.1) : Qt.lighter(color, 1.6)
    property bool joinInProgress: false

    id: preview
    height: contentCol.height + 20
    cornerRadius: guiSettings.radius

    Rectangle {
        anchors.fill: parent
        color: guiSettings.messageUserBackgroundColor
        border.width: 1
        border.color: guiSettings.isLightMode ? Qt.darker(color, 1.1) : Qt.lighter(color, 1.6)
    }

    Column {
        id: contentCol
        x: 10
        y: 10
        width: parent.width - 20
        spacing: 10
        visible: !joinLink.disabled && !joinLink.invalid

        RowLayout {
            id: chatInfo
            width: parent.width
            spacing: 10

            Avatar {
                Layout.preferredWidth: 50
                Layout.preferredHeight: Layout.preferredWidth
                userDid: userDid
                author: owner
                showGroupIcon: true

                onClicked: {
                    if (!owner.isNull())
                        skywalker.getDetailedProfile(owner.did)
                }
            }

            Column {
                Layout.fillWidth: true
                spacing: 0

                SkyCleanedText {
                    width: parent.width
                    elide: Text.ElideRight
                    font.bold: true
                    color: guiSettings.messageUserTextColor
                    plainText: joinLink.isNull() ? title : joinLink.name
                }

                AccessibleText {
                    width: parent.width
                    bottomPadding: 5
                    elide: Text.ElideRight
                    font.pointSize: guiSettings.scaledFont(7/8)
                    color: guiSettings.messageUserTextColor
                    text: qsTr(`Group chat ${joinLink.memberCount}/${joinLink.memberLimit} members`)
                }

                RowLayout {
                    width: parent.width

                    AccessibleText {
                        color: guiSettings.messageUserTextColor
                        text: qsTr("By")
                    }

                    AuthorNameAndStatus {
                        Layout.fillWidth: true
                        userDid: preview.userDid
                        author: owner
                        textColor: guiSettings.messageUserTextColor
                    }
                }

                AccessibleText {
                    width: parent.width
                    elide: Text.ElideRight
                    text: "@" + owner.handle
                    font.pointSize: guiSettings.scaledFont(7/8)
                    color: guiSettings.messageUserTextColor
                }

                AccessibleText {
                    topPadding: 5
                    elide: Text.ElideRight
                    font.pointSize: guiSettings.scaledFont(7/8)
                    color: guiSettings.messageUserTextColor
                    text: qsTr(`Requested to join: ${guiSettings.dateTimeIndication(joinLink.requestedAt)}`)
                    visible: joinLink.requestPending
                }
            }
        }

        SkyButton {
            width: parent.width
            text: getButtonText()
            enabled: buttonEnabled()
            onClicked: joinChat()
        }
    }

    Loader {
        anchors.fill: contentCol
        active: joinLink.disabled || joinLink.invalid

        sourceComponent: Item {

            RowLayout {
                width: parent.width
                anchors.centerIn: parent

                Avatar {
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: Layout.preferredWidth
                    userDid: userDid
                    unknownSvg: SvgFilled.group
                }

                AccessibleText {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: guiSettings.messageUserTextColor
                    font.bold: true
                    text: joinLink.disabled ? qsTr("Invite link is disabled") : qsTr("Invite link does not exist anymore")
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: joinInProgress
    }

    Connections {
        target: skywalker.chat

        function onJoinLinkPreviewOk(joinLink) {
            if (joinLink.code === preview.code)
                preview.joinLink = joinLink
        }

        function onRequestJoinPending(pendingJoinLink) {
            if (pendingJoinLink.code !== preview.code)
                return

            joinLink = pendingJoinLink

            if (joinInProgress) {
                joinInProgress = false
                skywalker.showStatusMessage(qsTr(`Request to join ${joinLink.name} sent`), QEnums.STATUS_LEVEL_INFO)
            }
        }

        function onRequestJoinJoined(joinedJoinLink) {
            if (joinedJoinLink.code !== preview.code)
                return

            joinLink = joinedJoinLink

            if (joinInProgress) {
                joinInProgress = false
                const convo = joinLink.getConvo()

                if (!convo.isNull())
                    root.openConvo(convo)
            }
        }

        function onRequestJoinFailed(code, error) {
            if (code !== preview.code)
                return

            if (joinInProgress) {
                joinInProgress = false
                skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
            }
        }

        function onWithdrawJoinRequestOk(withdrawnJoinLink) {
            if (withdrawnJoinLink.code !== preview.code)
                return;

            joinLink = withdrawnJoinLink

            if (joinInProgress) {
                joinInProgress = false
                skywalker.showStatusMessage(qsTr("Join request canceled"))
            }
        }

        function onWithdrawJoinRequestFailed(code, error) {
            if (code !== preview.code)
                return

            if (joinInProgress) {
                joinInProgress = false
                skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
            }
        }
    }

    function getButtonText() {
        if (joinLink.userIsMember)
            return qsTr("Open chat")

        if (joinLink.requestPending)
            return qsTr("Cancel request")

        if (joinLink.joinRule === QEnums.JOIN_RULE_FOLLOWED_BY_OWNER && !owner.viewer.followedBy)
            return qsTr(`Only people followed by the chat owner can join`)

        if (joinLink.requireApproval)
            return qsTr("Request to join")

        return qsTr("Join chat")
    }

    function buttonEnabled() {
        if (joinLink.userIsMember)
            return true

        if (joinLink.requestPending)
            return true

        if (joinLink.joinRule === QEnums.JOIN_RULE_FOLLOWED_BY_OWNER && !owner.viewer.followedBy)
            return false

        return true
    }

    function joinChat() {
        if (joinLink.userIsMember) {
            const convo = joinLink.getConvo()

            if (convo.isNull()) {
                console.warn("Convo not available:", joinLink.name, "uri:", uri)
                return
            }

            root.openConvo(convo)
            return
        }

        if (joinLink.requestPending) {
            cancelJoinRequest()
            return
        }

        if (joinLink.requireApproval) {
            requestJoin()
            return
        }

        join()
    }

    function join() {
        if (joinInProgress)
            return

        guiSettings.askYesNoQuestion(
            root,
            qsTr(`Do you want to join: ${joinLink.name} ?`),
            () => {
                joinInProgress = true
                skywalker.chat.requestJoin(joinLink)
            })
    }

    function requestJoin() {
        if (joinInProgress)
            return

        guiSettings.askYesNoQuestion(
            root,
            qsTr(`Do you want send a request to join: ${joinLink.name} ?`),
            () => {
                joinInProgress = true
                skywalker.chat.requestJoin(joinLink)
            })
    }

    function cancelJoinRequest() {
        if (joinInProgress)
            return

        guiSettings.askYesNoQuestion(
            root,
            qsTr(`Do you want to cancel you request to join: ${joinLink.name} ?`),
            () => {
                joinInProgress = true
                skywalker.chat.withdrawJoinRequest(joinLink)
            })
    }

    Component.onCompleted: {
        if (code)
            skywalker.chat.getJoinLinkPreview(code)
    }
}
