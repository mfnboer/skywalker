import QtQuick
import skywalker

FeedUtils {
    id: feedUtils

    onLikeFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onUndoLikeFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onInteractionsSent: skywalker.showStatusMessage(qsTr("Feedback sent"), QEnums.STATUS_LEVEL_INFO)
    onFailure: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
}
