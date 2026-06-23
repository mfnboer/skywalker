import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property convoview convo
    property var skywalker: root.getSkywalker()
    property basicprofile firstMember: convo.members.length > 0 ? convo.members[0].basicProfile : skywalker.getUserProfile()
    property bool isSideBar: false
    property int rightPadding: 0
    readonly property int margin: 10

    signal back

    id: headerRect
    width: parent.width
    height: guiSettings.headerHeight
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    Accessible.role: Accessible.Pane

    RowLayout {
        id: convoRow
        width: parent.width - headerRect.rightPadding
        height: parent.height
        spacing: 5

        SvgPlainButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            svg: SvgOutline.arrowBack
            accessibleName: qsTr("go back")
            onClicked: headerRect.back()
        }

        Avatar {
            id: avatar
            Layout.alignment: Qt.AlignVCenter
            width: parent.height - 10
            author: firstMember
            showGroupIcon: convo.kind === QEnums.CONVO_KIND_GROUP
            visible: !isSideBar
            onClicked: skywalker.getDetailedProfile(firstMember.did)
        }

        Column {
            id: convoColumn
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.leftMargin: 5
            Layout.rightMargin: margin
            spacing: 0

            SkyCleanedTextLine {
                width: parent.width
                topPadding: 3
                Layout.fillHeight: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.headerTextColor
                plainText: convo.title
            }

            AccessibleText {
                width: parent.width
                Layout.preferredHeight: visible ? implicitHeight : 0
                color: guiSettings.handleColor
                font.pointSize: guiSettings.scaledFont(7/8)
                elide: Text.ElideRight
                text: `@${firstMember.handle}`
                visible: convo.members.length <= 1
            }

            Row {
                width: parent.width
                spacing: 3
                clip: true

                Repeater {
                    id: activeMembers
                    model: convo.members.slice(1, 1 + parent.calcNumAvatars())

                    Avatar {
                        required property chatbasicprofile modelData

                        width: guiSettings.avatarSmallWidth
                        author: modelData.basicProfile
                        showFollowingStatus: false
                        onClicked: skywalker.getDetailedProfile(author.did)
                    }
                }
                AccessibleText {
                    anchors.verticalCenter: parent.verticalCenter
                    text: `+${(convo.group.memberCount - activeMembers.count - 1 )}`
                    visible: convo.group.memberCount > activeMembers.count + 1
                }

                visible: convo.members.length > 1

                function calcNumAvatars() {
                    const maxAvatars = parent.width / (guiSettings.avatarSmallWidth + spacing)

                    if (convo.group.memberCount - 1 <= maxAvatars)
                        return maxAvatars

                    return maxAvatars - 1 // leavespace for "+N"
                }
            }
        }
    }

}
