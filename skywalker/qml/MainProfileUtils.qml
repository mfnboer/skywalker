import QtQuick
import skywalker

ProfileUtils {
    required property StatusPopup statusPopup

    id: profileUtils

    onSetPinnedPostOk: statusPopup.show(qsTr("Post pinned"), QEnums.STATUS_LEVEL_INFO, 2)
    onSetPinnedPostFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onClearPinnedPostOk: statusPopup.show(qsTr("Post unpinned"), QEnums.STATUS_LEVEL_INFO, 2)
    onClearPinnedPostFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
}
