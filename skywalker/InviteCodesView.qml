import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property list<InviteCode> codes
    required property bool failedToLoad

    signal closed
    signal authorClicked(string did)

    id: inviteCodeList
    width: parent.width
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    model: codes
    flickDeceleration: GuiSettings.flickDeceleration
    maximumFlickVelocity: GuiSettings.maxFlickVelocity
    pixelAligned: GuiSettings.flickPixelAligned

    header: SimpleHeader {
        text: qsTr("Invite Codes")
        onBack: inviteCodeList.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: Rectangle {
        required property var modelData // class InviteCode

        id: codeEntry
        width: inviteCodeList.width
        height: codeRow.height + usedByRow.height
        color: "transparent"

        RowLayout {
            id: codeRow
            width: parent.width

            Column {
                Layout.fillWidth: true

                Text {
                    width: parent.width
                    leftPadding: 10
                    topPadding: 10
                    font.strikeout: !modelData.available || modelData.disabled
                    color: modelData.disabled ? Material.color(Material.Grey) : GuiSettings.textColor
                    text: modelData.code
                }
                Text {
                    width: parent.width
                    leftPadding: 10
                    font.pointSize: GuiSettings.scaledFont(7/8)
                    color: Material.color(Material.Grey)
                    text: modelData.createdAt.toLocaleDateString(Qt.locale(), Locale.LongFormat)
                }
            }

            SvgButton {
                id: copyButton
                svg: SvgOutline.copy
                accessibleName: qsTr("copy invite code")
                iconColor: GuiSettings.textColor
                Material.background: "transparent"
                visible: modelData.available && !modelData.disabled
                onClicked: modelData.copyToClipboard()
            }
        }
        GridLayout {
            id: usedByRow
            anchors.top: codeRow.bottom
            width: parent.width
            columns: 2
            rowSpacing: 2

            // Avatar
            Rectangle {
                id: avatar
                Layout.rowSpan: 3
                width: 65
                height: avatarImg.height + 20
                Layout.fillHeight: true
                color: "transparent"
                visible: modelData.used

                Avatar {
                    id: avatarImg
                    x: parent.x + 8
                    y: parent.y + 10
                    width: parent.width - 12
                    height: width
                    author: modelData.usedBy
                    onClicked: authorClicked(modelData.usedByDid)
                }
            }

            SkyCleanedTextLine {
                id: nameText
                topPadding: 5
                Layout.fillWidth: true
                elide: Text.ElideRight
                color: GuiSettings.textColor
                plainText: modelData.usedBy.name
                visible: modelData.used
            }

            Text {
                id: handleText
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.pointSize: GuiSettings.scaledFont(7/8)
                color: GuiSettings.handleColor
                text: `@${modelData.usedBy.handle}`
                visible: modelData.used
            }

            Text {
                id: dateText
                Layout.fillWidth: true
                Layout.fillHeight: true
                font.pointSize: GuiSettings.scaledFont(7/8)
                color: Material.color(Material.Grey)
                text: modelData.usedAt.toLocaleDateString(Qt.locale(), Locale.LongFormat)
                visible: modelData.used
            }

            Rectangle {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: GuiSettings.separatorColor
            }
        }
        MouseArea {
            z: -1
            anchors.fill: parent
            enabled: modelData.used
            onClicked: authorClicked(modelData.usedByDid)
        }
    }

    Text {
        leftPadding: 10
        y: parent.headerItem ? parent.headerItem.height + 10 : 0
        width: parent.width
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        color: GuiSettings.textColor
        text: qsTr("Could not retrieve invite codes. To retrieve invite codes you need to sign in with your real password, not an app password.")
        visible: failedToLoad
    }

    Text {
        leftPadding: 10
        y: parent.headerItem ? parent.headerItem.height + 10 : 0
        width: parent.width
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        color: GuiSettings.textColor
        text: qsTr("You do not yet have any invide codes.")
        visible: !failedToLoad && inviteCodeList.count === 0
    }

}
