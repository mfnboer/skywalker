import QtQuick
import skywalker

GraphUtils {
    required property StatusPopup statusPopup

    id: graphUtils

    onGetListOk: (did, list, viewPosts) => { // qmllint disable signal-handler-parameters
        if (viewPosts)
            root.viewList(list, did)
        else
            root.viewListFeedDescription(list, did)
    }
    onGetListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

    onBlockOk: (uri, expiresAt) => {
        if (isNaN(expiresAt.getTime()))
            statusPopup.show(qsTr("Blocked"), QEnums.STATUS_LEVEL_INFO, 2)
        else
            statusPopup.show(qsTr(`Blocked till ${guiSettings.expiresIndication(expiresAt)}`), QEnums.STATUS_LEVEL_INFO, 2)
    }

    onBlockFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
}
