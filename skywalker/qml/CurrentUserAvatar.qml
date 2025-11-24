import QtQuick
import skywalker

Avatar {
                property Skywalker skywalker: root.getSkywalker(userDid)

                author: skywalker.getUserSettings().getUser(skywalker.getUserDid())
                showFollowingStatus: false
                onClicked:  {
                                if (root.isActiveUser(userDid)) {
                                                skywalker.showStatusMessage(qsTr("Yes, you're gorgeous"), QEnums.STATUS_LEVEL_INFO)
                                } else {
                                                skywalker.showStatusMessage(
                                                                                                qsTr(`You are using <font color="${guiSettings.linkColor}">@${author.handle}</font> to interact with this page.`),
                                                                                                QEnums.STATUS_LEVEL_INFO, 5)
                                }
                }
}
