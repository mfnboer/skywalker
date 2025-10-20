import QtQuick
import skywalker

FeedUtils {
    required property StatusPopup statusPopup

    id: feedUtils

    onLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onUndoLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onInteractionsSent: statusPopup.show(qsTr("Feedback sent"), QEnums.STATUS_LEVEL_INFO)
    onFailure: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
}
