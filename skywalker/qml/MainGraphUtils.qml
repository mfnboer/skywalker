import QtQuick
import skywalker

GraphUtils {
    id: graphUtils

    onGetListOk: (did, list, viewPosts) => { // qmllint disable signal-handler-parameters
        if (viewPosts)
            root.viewList(list, did)
        else
            root.viewListFeedDescription(list, did)
    }
    onGetListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

    onBlockOk: (uri, expiresAt) => {
        if (isNaN(expiresAt.getTime()))
            skywalker.showStatusMessage(qsTr("Blocked"), QEnums.STATUS_LEVEL_INFO, 2)
        else
            skywalker.showStatusMessage(qsTr(`Blocked till ${guiSettings.expiresIndication(expiresAt)}`), QEnums.STATUS_LEVEL_INFO, 2)
    }

    onBlockFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }

    onMuteOk: (expiresAt) => {
        if (isNaN(expiresAt.getTime()))
            skywalker.showStatusMessage(qsTr("Muted"), QEnums.STATUS_LEVEL_INFO, 2)
        else
            skywalker.showStatusMessage(qsTr(`Muted till ${guiSettings.expiresIndication(expiresAt)}`), QEnums.STATUS_LEVEL_INFO, 2)
    }

    onMuteFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }
}
