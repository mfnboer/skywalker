import QtQuick
import skywalker

Avatar {
    Text {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: -5
        text: skywalker.userAccountStatus.standingSymbol
        visible: !skywalker.userAccountStatus.isNull() && skywalker.userAccountStatus.standing !== QEnums.OZONE_ACCOUNT_STANDING_GOOD
    }
}
