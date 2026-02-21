import QtQuick
import skywalker

DraftPosts {
    signal draftSaved

    onSaveDraftPostOk: {
        skywalker.showStatusMessage(qsTr("Saved post as draft"), QEnums.STATUS_LEVEL_INFO)
        draftSaved()
    }

    onLoadDraftPostsFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onSaveDraftPostFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    onUploadingImage: (seq) => skywalker.showStatusMessage(qsTr(`Uploading image #${seq}`), QEnums.STATUS_LEVEL_INFO)
    onDeleteDraftFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

    function deleteDraft(index) {
        guiSettings.askYesNoQuestion(
                    root,
                    qsTr("Do you really want to delete your draft?"),
                    () => removeDraftPost(index))
    }
}
