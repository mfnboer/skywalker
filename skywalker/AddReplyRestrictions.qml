import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property bool restrictReply
    required property bool allowMentioned
    required property bool allowFollowing
    required property list<bool> allowLists
    required property list<int> allowListIndexes
    required property int listModelId
    property list<bool> duplicateList: [false, false, false]
    property list<var> comboBoxList: [null, null, null]

    id: restrictionDialog
    width: parent.width
    contentHeight: restrictionColumn.height
    title: qsTr("Who can reply?")
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

    onAllowListsChanged: {
        allowLists.forEach((allow) => {
                            if (allow)
                                restrictReply = true
                        })

        checkUniqueLists()
    }

    onAllowListIndexesChanged: checkUniqueLists()

    function checkUniqueLists() {
        let duplicates = false
        let lists = []
        duplicateList = [false, false, false]

        for (let i = 0; i < allowLists.length; ++i) {
            if (allowLists[i]) {
                const index = allowListIndexes[i]

                if (lists.includes(index)) {
                    duplicateList[i] = true
                    duplicates = true
                    continue
                }

                lists.push(index)
            }
        }

        let okButton = standardButton(Dialog.Ok)

        if (okButton)
            okButton.enabled = !duplicates
    }

    Column {
        id: restrictionColumn
        width: parent.width

        CheckBox {
            checked: !restrictReply
            text: qsTr("Everyone")
            onCheckedChanged: {
                restrictReply = !checked

                if (checked) {
                    allowMentioned = false
                    allowFollowing = false
                    allowLists = [false, false, false]
                }
            }
        }
        CheckBox {
            checked: restrictReply && !allowMentioned && !allowFollowing && !allowLists[0] && !allowLists[1] && !allowLists[2]
            text: qsTr("Nobody")
            onCheckedChanged: {
                if (checked) {
                    restrictReply = true
                    allowMentioned = false
                    allowFollowing = false
                    allowLists = [false, false, false]
                }
            }
        }
        CheckBox {
            checked: allowMentioned
            text: qsTr("Users mentioned in your post")
            onCheckedChanged: {
                allowMentioned = checked

                if (checked)
                    restrictReply = true
            }
        }
        CheckBox {
            checked: allowFollowing
            text: qsTr("Users you follow")
            onCheckStateChanged: {
                allowFollowing = checked

                if (checked)
                    restrictReply = true
            }
        }

        Repeater {
            property alias restrictReply: restrictionDialog.restrictReply

            width: parent.width
            model: allowLists

            Row {
                required property int index

                width: parent.width
                visible: listComboBox.count > index

                CheckBox {
                    id: allowListCheckBox
                    checked: allowLists[parent.index]
                    text: qsTr("Allow users from list:")
                    onCheckStateChanged: allowLists[parent.index] = checked
                }
                PagingComboBox {
                    id: listComboBox
                    width: parent.width - allowListCheckBox.width
                    height: allowListCheckBox.height
                    model: skywalker.getListListModel(listModelId)
                    valueRole: "listUri"
                    textRole: "listName"
                    inProgress: skywalker.getListListInProgress
                    bottomOvershootFun: () => skywalker.getListListNextPage(listModelId)
                    initialIndex: allowListIndexes[parent.index]
                    backgroundColor: duplicateList[parent.index] ? guiSettings.errorColor : Material.dialogColor
                    enabled: allowLists[parent.index]

                    onCurrentIndexChanged: allowListIndexes[parent.index] = currentIndex

                    Component.onCompleted: comboBoxList[parent.index] = listComboBox
                }
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
