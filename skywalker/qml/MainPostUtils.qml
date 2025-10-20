import QtQuick
import skywalker

PostUtils {
    required property StatusPopup statusPopup
    property list<int> allowListIndexes: [0, 1, 2]
    property list<bool> allowLists: [false, false, false]
    property list<string> allowListUris: []
    property list<listviewbasic> allowListViews: []
    property string checkPostUri
    property string checkPostCid
    property var checkPostCallback

    id: postUtils

    onRepostOk: statusPopup.show(qsTr("Reposted"), QEnums.STATUS_LEVEL_INFO, 2)
    onRepostFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onRepostProgress: (msg) => statusPopup.show(qsTr("Reposting"), QEnums.STATUS_LEVEL_INFO)
    onUndoRepostOk: statusPopup.show(qsTr("Repost undone"), QEnums.STATUS_LEVEL_INFO, 2)
    onUndoRepostFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onUndoLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onMuteThreadOk: statusPopup.show(qsTr("You will no longer receive notifications for this thread"), QEnums.STATUS_LEVEL_INFO, 5);
    onMuteThreadFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onUnmuteThreadOk: statusPopup.show(qsTr("You will receive notifications for this thread"), QEnums.STATUS_LEVEL_INFO, 5);
    onUnmuteThreadFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onThreadgateOk: statusPopup.show(qsTr("Reply settings changed"), QEnums.STATUS_LEVEL_INFO, 2)
    onThreadgateFailed: (error) =>  statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onUndoThreadgateOk: statusPopup.show(qsTr("Reply settings changed"), QEnums.STATUS_LEVEL_INFO, 2)
    onUndoThreadgateFailed: (error) =>  statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onPostgateOk: statusPopup.show(qsTr("Quote settings changed"), QEnums.STATUS_LEVEL_INFO, 2)
    onPostgateFailed: (error) =>  statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    onUndoPostgateOk: statusPopup.show(qsTr("Quote settings changed"), QEnums.STATUS_LEVEL_INFO, 2)
    onUndoPostgateFailed: (error) =>  statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

    onDetachQuoteOk: (detached) => {
        if (detached)
            statusPopup.show(qsTr("Quote detached"), QEnums.STATUS_LEVEL_INFO, 2)
        else
            statusPopup.show(qsTr("Quote re-attached"), QEnums.STATUS_LEVEL_INFO, 2)
    }

    onDetachQuoteFailed: (error) =>  statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

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
        statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function checkPost(uri, cid, callback) {
        checkPostUri = uri
        checkPostCid = cid
        checkPostCallback = callback
        postUtils.checkPostExists(uri, cid)
    }
}
