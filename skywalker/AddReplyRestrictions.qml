import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property bool restrictReply
    required property bool allowMentioned
    required property bool allowFollowing
    required property bool allowList1
    required property int allowList1Index
    required property bool allowList2
    required property int allowList2Index
    required property bool allowList3
    required property int allowList3Index
    required property int listModelId

    width: parent.width
    contentHeight: restrictionColumn.height
    title: qsTr("Who can reply?")
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

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
                    allowList1 = false
                    allowList2 = false
                    allowList3 = false
                }
            }
        }
        CheckBox {
            checked: restrictReply && !allowMentioned && !allowFollowing && !allowList1 && !allowList2 && !allowList3
            text: qsTr("Nobody")
            onCheckedChanged: {
                if (checked) {
                    restrictReply = true
                    allowMentioned = false
                    allowFollowing = false
                    allowList1 = false
                    allowList2 = false
                    allowList3 = false
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
        Row {
            width: parent.width
            visible: list1.count > 0

            CheckBox {
                id: allowList1CheckBox
                checked: allowList1
                text: qsTr("Allow users from list:")
                onCheckStateChanged: {
                    allowList1 = checked

                    if (checked)
                        restrictReply = true
                }
            }
            PagingComboBox {
                id: list1
                width: parent.width - allowList1CheckBox.width
                height: allowList1CheckBox.height
                model: skywalker.getListListModel(listModelId)
                valueRole: "listUri"
                textRole: "listName"
                inProgress: skywalker.getListListInProgress
                bottomOvershootFun: () => skywalker.getListListNextPage(listModelId)
                initialIndex: allowList1Index
                enabled: allowList1

                onCurrentIndexChanged: allowList1Index = currentIndex
            }
        }
        Row {
            width: parent.width
            visible: list2.count > 1

            CheckBox {
                id: allowList2CheckBox
                checked: allowList2
                text: qsTr("Allow users from list:")
                onCheckStateChanged: {
                    allowList2 = checked

                    if (checked)
                        restrictReply = true
                }
            }
            PagingComboBox {
                id: list2
                width: parent.width - allowList1CheckBox.width
                height: allowList1CheckBox.height
                model: skywalker.getListListModel(listModelId)
                valueRole: "listUri"
                textRole: "listName"
                inProgress: skywalker.getListListInProgress
                bottomOvershootFun: () => skywalker.getListListNextPage(listModelId)
                initialIndex: allowList2Index
                enabled: allowList2

                onCurrentIndexChanged: allowList2Index = currentIndex
            }
        }
        Row {
            width: parent.width
            visible: list3.count > 2

            CheckBox {
                id: allowList3CheckBox
                checked: allowList3
                text: qsTr("Allow users from list:")
                onCheckStateChanged: {
                    allowList3 = checked

                    if (checked)
                        restrictReply = true
                }
            }
            PagingComboBox {
                id: list3
                width: parent.width - allowList1CheckBox.width
                height: allowList1CheckBox.height
                model: skywalker.getListListModel(listModelId)
                valueRole: "listUri"
                textRole: "listName"
                inProgress: skywalker.getListListInProgress
                bottomOvershootFun: () => skywalker.getListListNextPage(listModelId)
                initialIndex: allowList3Index
                enabled: allowList3

                onCurrentIndexChanged: allowList3Index = currentIndex
            }
        }
    }
}
