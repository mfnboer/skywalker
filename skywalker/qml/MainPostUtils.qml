import QtQuick
import skywalker

PostUtils {
    property list<int> allowListIndexes: [0, 1, 2]
    property list<bool> allowLists: [false, false, false]
    property list<string> allowListUris: []
    property list<listviewbasic> allowListViews: []
    property string checkPostUri
    property string checkPostCid
    property var checkPostCallback

    id: postUtils

    onRepostOk: skywalker.showStatusMessage(qsTr("Reposted"), QEnums.STATUS_LEVEL_INFO, 2)
    onRepostFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onRepostProgress: (msg) => skywalker.showStatusMessage(qsTr("Reposting"), QEnums.STATUS_LEVEL_INFO)
    onUndoRepostOk: skywalker.showStatusMessage(qsTr("Repost undone"), QEnums.STATUS_LEVEL_INFO, 2)
    onUndoRepostFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onLikeFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onUndoLikeFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onMuteThreadOk: skywalker.showStatusMessage(qsTr("You will no longer receive notifications for this thread"), QEnums.STATUS_LEVEL_INFO, 5);
    onMuteThreadFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onUnmuteThreadOk: skywalker.showStatusMessage(qsTr("You will receive notifications for this thread"), QEnums.STATUS_LEVEL_INFO, 5);
    onUnmuteThreadFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onThreadgateOk: skywalker.showStatusMessage(qsTr("Reply settings changed"), QEnums.STATUS_LEVEL_INFO, 2)
    onThreadgateFailed: (error) =>  skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onUndoThreadgateOk: skywalker.showStatusMessage(qsTr("Reply settings changed"), QEnums.STATUS_LEVEL_INFO, 2)
    onUndoThreadgateFailed: (error) =>  skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onPostgateOk: skywalker.showStatusMessage(qsTr("Quote settings changed"), QEnums.STATUS_LEVEL_INFO, 2)
    onPostgateFailed: (error) =>  skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onUndoPostgateOk: skywalker.showStatusMessage(qsTr("Quote settings changed"), QEnums.STATUS_LEVEL_INFO, 2)
    onUndoPostgateFailed: (error) =>  skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

    onDetachQuoteOk: (detached) => {
        if (detached)
            skywalker.showStatusMessage(qsTr("Quote detached"), QEnums.STATUS_LEVEL_INFO, 2)
        else
            skywalker.showStatusMessage(qsTr("Quote re-attached"), QEnums.STATUS_LEVEL_INFO, 2)
    }

    onDetachQuoteFailed: (error) =>  skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

    function setAllowListUris(replyRestrictionLists) {
        allowLists = [false, false, false]
        allowListUris = []

        for (let i = 0; i < Math.min(replyRestrictionLists.length, 3); ++i) {
            allowLists[i] = true
            allowListUris.push(replyRestrictionLists[i].uri)
        }
    }

    onCheckPostExistsOk: (uri, cid) => {
        if (uri !== checkPostUri || cid !== checkPostCid)
            return

        checkPostUri = ""
        checkPostCid = ""
        checkPostCallback()
        checkPostCallback = null
    }

    onCheckPostExistsFailed: (uri, cid, error) => {
        if (uri !== checkPostUri || cid !== checkPostCid)
            return

        checkPostUri = ""
        checkPostCid = ""
        checkPostCallback = null
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function checkPost(uri, cid, callback) {
        checkPostUri = uri
        checkPostCid = cid
        checkPostCallback = callback
        postUtils.checkPostExists(uri, cid)
    }
}
