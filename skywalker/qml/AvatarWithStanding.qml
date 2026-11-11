import QtQuick
import skywalker

Avatar {
    readonly property OzoneInbox ozoneInbox: skywalker.ozoneInbox

    Text {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: -5
        text: ozoneInbox.userAccountStatus.standingSymbol
        visible: !ozoneInbox.userAccountStatus.isNull() && ozoneInbox.userAccountStatus.standing !== QEnums.OZONE_ACCOUNT_STANDING_GOOD
    }

    BadgeCounter {
        y: 0
        counter: ozoneInbox.unreadReports + ozoneInbox.unreadSubjects
    }
}
