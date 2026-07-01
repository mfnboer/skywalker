import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyDialog {
    required property convoview convo
    property groupconvo group: convo.group
    property joinlinkview joinLink: group.joinLinkView
    readonly property bool userIsOwner: convo.getMember(skywalker.getUserDid()).groupMember.role === QEnums.CONVO_MEMBER_ROLE_OWNER
    property Skywalker skywalker: root.getSkywalker()

    id: dialog
    width: parent.width - 40
    contentHeight: contentColumn.height
    standardButtons: Dialog.Close
    anchors.centerIn: parent

    title: qsTr("Invite link")

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: contentColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: contentColumn
            width: parent.width

            AccessibleText {
                width: parent.width
                bottomPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(9/8)
                text: convo.group.name
            }

            TextAndSvgRow {
                id: copyLink
                width: parent.width
                svg: SvgOutline.copy
                text: convo.getInviteLinkUrl()
                enabled: joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_ENABLED
                onClicked: skywalker.getShareUtils().copyToClipboard(copyLink.text)
            }

            AccessibleText {
                width: parent.width
                bottomPadding: 10
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: Material.color(Material.Grey)
                text: qsTr(`Created at ${joinLink.createdAt.toLocaleString(Qt.locale(), Locale.ShortFormat)}`)
            }

            TextAndSvgRow {
                width: parent.width
                svg: SvgOutline.edit
                wrapMode: Text.Wrap
                text: getJoinText()
                visible: joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_ENABLED
                onClicked: root.createOrEditJoinLink(convo, false)
            }

            AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                font.bold: true
                text: qsTr("Invite link is disabled")
                visible: joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_DISABLED
            }

            Item {
                width: parent.width
                height: 10
            }

            RowLayout {
                width: parent.width
                spacing: 0

                SkyButton {
                    Layout.fillWidth: true
                    text: joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_ENABLED ? qsTr("Disable") : qsTr("Enable")
                    visible: userIsOwner
                    onClicked: {
                        if (joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_ENABLED)
                            disableJoinLink()
                        if (joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_DISABLED)
                            skywalker.chat.enableJoinLink(convo.id)
                    }
                }
                SkyButton {
                    Layout.fillWidth: true
                    text: qsTr("Generate new link")
                    visible: userIsOwner && joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_DISABLED
                    onClicked: root.createOrEditJoinLink(convo, false)
                }
                SkyButton {
                    Layout.fillWidth: true
                    text: qsTr("Post")
                    visible: joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_ENABLED
                    onClicked: {
                        root.composePost(convo.getInviteLinkUrl())
                        dialog.reject()
                    }
                }
                SkyButton {
                    Layout.fillWidth: true
                    text: qsTr("Share")
                    visible: joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_ENABLED
                    onClicked: skywalker.getShareUtils().shareConvo(convo)
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: skywalker.chat.convoUpdateInProgress
    }

    Connections {
        target: skywalker.chat

        function onConvoUpdated(convo) {
            if (convo.id === dialog.convo.id)
                dialog.convo = convo
        }
    }

    function getJoinText() {
        const owner = convo.getOwner()

        switch (joinLink.joinRule) {
        case QEnums.JOIN_RULE_ANYONE:
            if (joinLink.requireApproval && userIsOwner)
                return qsTr("Anyone can request to join")
            else
                return qsTr("Anyone can join")
        case QEnums.JOIN_RULE_FOLLOWED_BY_OWNER:
            if (joinLink.requireApproval) {
                if (userIsOwner)
                    return qsTr("People you follow can request to join")
                else
                    return qsTr(`People ${owner.name} follows can request to join`)
            } else {
                if (userIsOwner)
                    return qsTr("People you follow can join")
                else
                    return qsTr(`People ${owner.name} follows can join`)
            }
        default:
            return qsTr("Unsupported join rule")
        }
    }

    function disableJoinLink() {
        guiSettings.askYesNoQuestion(
            root,
            qsTr("Anyone who has the link will no longer be able to join. Do you want to disable the link?"),
            () => skywalker.chat.disableJoinLink(convo.id)
        )
    }
}
