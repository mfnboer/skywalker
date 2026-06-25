import QtQuick
import skywalker

Row {
    required property convoview convo

    spacing: 3

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
        const maxAvatars = width / (guiSettings.avatarSmallWidth + spacing)

        if (convo.group.memberCount - 1 <= maxAvatars)
            return maxAvatars

        return maxAvatars - 1 // leavespace for "+N"
    }
}
