import QtQuick
import skywalker

Avatar {
                required property string userDid
                author: root.getSkywalker(userDid).getUserSettings().getUser(userDid)
                showActivityStatus: false
                onClicked: root.getSkywalker(userDid).showStatusMessage(
                               qsTr(`You are using <font color="${guiSettings.linkColor}">@${author.handle}</font> to interact with this page.`),
                               QEnums.STATUS_LEVEL_INFO, 5)
}
