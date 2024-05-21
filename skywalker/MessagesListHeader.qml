import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property convoview convo
    property var skywalker: root.getSkywalker()
    property basicprofile firstMember: convo.members.length > 0 ? convo.members[0].basicProfile : skywalker.getUserProfile()
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
        width: parent.width
        height: parent.height
        spacing: 5

        SvgButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            svg: svgOutline.arrowBack
            accessibleName: qsTr("go back")
            onClicked: headerRect.back()
        }

        Avatar {
            id: avatar
            Layout.alignment: Qt.AlignVCenter
            width: parent.height - 10
            height: width
            avatarUrl: guiSettings.authorVisible(firstMember) ? firstMember.avatarUrl : ""
            isModerator: firstMember.associated.isLabeler
            onClicked: skywalker.getDetailedProfile(firstMember.did)
        }

        Column {
            id: convoColumn
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 5
            Layout.rightMargin: margin
            spacing: 3

            SkyCleanedText {
                width: parent.width
                Layout.fillHeight: true
                topPadding: 5
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.headerTextColor
                plainText: convo.memberNames
            }

            AccessibleText {
                width: parent.width
                Layout.preferredHeight: visible ? implicitHeight : 0
                color: guiSettings.handleColor
                font.pointSize: guiSettings.scaledFont(7/8)
                text: `@${firstMember.handle}`
                visible: convo.members.length <= 1
            }

            Row {
                width: parent.width
                spacing: 3

                Repeater {
                    model: convo.members.slice(1)

                    Avatar {
                        required property chatbasicprofile modelData

                        width: 25
                        height: width
                        avatarUrl: guiSettings.authorVisible(modelData.basicProfile) ? modelData.basicProfile.avatarUrl : ""
                        isModerator: modelData.basicProfile.associated.isLabeler
                    }
                }

                visible: convo.members.length > 1
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
