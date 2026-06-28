import QtQuick
import QtQuick.Controls
import skywalker

SkyDialog {
    property convoview convo
    property groupconvo group: convo.group
    property joinlinkview joinLink: group.joinLinkView
    property Skywalker skywalker: root.getSkywalker()
    property int joinRule: joinLink.joinRule
    property bool requireApproval: joinLink.requireApproval

    id: dialog
    width: parent.width - 40
    contentHeight: contentColumn.height
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

    title: joinLink.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_DISABLED ? qsTr("Generate invite link") : qsTr("Edit invite link")

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: contentColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: contentColumn
            spacing: 10
            width: parent.width

            AccessibleText {
                width: parent.width
                bottomPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(9/8)
                text: convo.group.name
            }

            AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("An invite link lets people join this group chat without being added directly. You control who can join. You can disable the link at any time.")
            }

            AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("Choose who can join this group chat.")
            }

            SkyRoundRadioButton {
                width: parent.width
                text: qsTr("Anyone can join")
                checked: joinRule === QEnums.JOIN_RULE_ANYONE
                onCheckedChanged: {
                    if (checked)
                        joinRule = QEnums.JOIN_RULE_ANYONE
                }
            }
            SkyRoundRadioButton {
                width: parent.width
                text: qsTr("People you follow can join")
                checked: joinRule === QEnums.JOIN_RULE_FOLLOWED_BY_OWNER
                onCheckedChanged: {
                    if (checked)
                        joinRule = QEnums.JOIN_RULE_FOLLOWED_BY_OWNER
                }
            }

            AccessibleCheckBox {
                width: parent.width
                text: qsTr("You will approve/reject requests to join")
                checked: requireApproval
                onCheckedChanged: requireApproval = checked
            }
        }
    }
}
