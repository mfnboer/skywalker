import QtQuick
import skywalker

ProfileUtils {
    id: profileUtils

    onSetPinnedPostOk: skywalker.showStatusMessage(qsTr("Post pinned"), QEnums.STATUS_LEVEL_INFO, 2)
    onSetPinnedPostFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onClearPinnedPostOk: skywalker.showStatusMessage(qsTr("Post unpinned"), QEnums.STATUS_LEVEL_INFO, 2)
    onClearPinnedPostFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
}
