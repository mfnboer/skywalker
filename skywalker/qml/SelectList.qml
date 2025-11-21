import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Dialog {
    required property int listModelId
    property Skywalker skywalker: root.getSkywalker()

    signal newList

    id: dialog
    width: parent.width - 20
    contentHeight: listComboBox.height + newListButton.height + 10
    modal: true
    title: qsTr("Select list")
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    PagingComboBox {
        id: listComboBox
        width: parent.width
        backgroundColor: guiSettings.textInputBackgroundColor
        model: skywalker.getListListModel(listModelId)
        valueRole: "listUri"
        textRole: "listName"
        inProgress: model?.getFeedInProgress
        bottomOvershootFun: () => skywalker.getListListNextPage(listModelId)
    }

    SkyButton {
        id: newListButton
        anchors.top: listComboBox.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        height: 40
        text: qsTr("New list")
        onClicked: dialog.newList()
    }

    function getList() {
        return listComboBox.model.getEntry(listComboBox.currentIndex)
    }
}
