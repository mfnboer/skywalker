import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Dialog {
    required property string rootUri
    required property string postUri
    required property bool restrictReply
    required property bool allowMentioned
    required property bool allowFollower
    required property bool allowFollowing
    required property list<bool> allowLists
    required property list<int> allowListIndexes
    required property list<string> allowListUrisFromDraft
    required property int listModelId
    property list<bool> duplicateList: [false, false, false]
    property bool prevAllowQuoting: true
    property bool allowQuoting: true
    property postgate postgate
    property bool postgateReceived: false
    property bool isThreadFromUser: false
    property bool saveAsDefault: false

    id: restrictionDialog
    width: parent.width
    contentHeight: restrictionColumn.height
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    onAccepted: {
        if (saveAsDefault)
            saveRestrictionsAsDefault()
    }

    onAllowListsChanged: {
        restrictReply = hasAllowLists()
        checkUniqueLists()
    }

    onAllowListIndexesChanged: checkUniqueLists()

    function hasAllowLists() {
        let hasLists = false

        allowLists.forEach((allow) => {
            if (allow)
                hasLists = true
        })

        return hasLists
    }

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

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: restrictionColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: restrictionColumn
            width: parent.width

            AccessibleText {
                width: parent.width
                padding: 10
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                text: qsTr("Quote settings")
            }

            AccessibleCheckBox {
                text: qsTr("Allow others to quote this post")
                checked: allowQuoting
                enabled: postgateReceived
                onCheckedChanged: allowQuoting = checked
            }

            AccessibleText {
                width: parent.width
                padding: 10
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                text: qsTr("Who can reply to this thread?")
            }

            AccessibleCheckBox {
                checked: !restrictReply
                text: qsTr("Everyone")
                visible: isThreadFromUser
                onCheckedChanged: {
                    restrictReply = !checked

                    if (checked) {
                        allowMentioned = false
                        allowFollower = false
                        allowFollowing = false
                        allowLists = [false, false, false]
                    }
                }
            }
            AccessibleCheckBox {
                checked: restrictReply && !allowMentioned && !allowFollower && !allowFollowing && !allowLists[0] && !allowLists[1] && !allowLists[2]
                text: qsTr("Nobody")
                visible: isThreadFromUser
                onCheckedChanged: {
                    if (checked) {
                        restrictReply = true
                        allowMentioned = false
                        allowFollower = false
                        allowFollowing = false
                        allowLists = [false, false, false]
                    }
                }
            }
            AccessibleCheckBox {
                checked: allowMentioned
                text: qsTr("Users mentioned in your post")
                visible: isThreadFromUser
                onCheckedChanged: {
                    allowMentioned = checked

                    if (checked)
                        restrictReply = true
                }
            }
            AccessibleCheckBox {
                checked: allowFollower
                text: qsTr("Users following you")
                visible: isThreadFromUser
                onCheckStateChanged: {
                    allowFollower = checked

                    if (checked)
                        restrictReply = true
                }
            }
            AccessibleCheckBox {
                checked: allowFollowing
                text: qsTr("Users you follow")
                visible: isThreadFromUser
                onCheckStateChanged: {
                    allowFollowing = checked

                    if (checked)
                        restrictReply = true
                }
            }

            Repeater {
                property alias restrictReply: restrictionDialog.restrictReply

                id: listRestrictions
                width: parent.width
                model: allowLists.length
                visible: isThreadFromUser

                function available() {
                    const item = itemAt(0)
                    return item && item.visible
                }

                Row {
                    required property int index

                    width: parent.width
                    visible: listComboBox.count > index && isThreadFromUser

                    AccessibleCheckBox {
                        id: allowListCheckBox
                        checked: allowLists[parent.index]
                        text: qsTr("Users from list:")
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
                        findValue: getListUriFromDraft(parent.index)
                        backgroundColor: duplicateList[parent.index] ? guiSettings.errorColor : Material.dialogColor
                        enabled: allowLists[parent.index]

                        onCurrentIndexChanged: allowListIndexes[parent.index] = currentIndex
                        onValueFound: allowLists[parent.index] = true
                    }
                }
            }

            AccessibleText {
                x: 10
                width: parent.width - 20
                wrapMode: Text.Wrap
                font.italic: true
                text: qsTr("User lists can also be used to restrict who can reply. You have no user lists at this moment.");
                visible: isThreadFromUser && (listRestrictions.count === 0 || !listRestrictions.available())
            }

            AccessibleText {
                x: 10
                width: parent.width - 20
                wrapMode: Text.Wrap
                font.italic: true
                text: qsTr("You cannot restrict replies as this is not your thread.")
                visible: !isThreadFromUser
            }

            AccessibleText {
                width: parent.width
                padding: 10
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                text: qsTr("Default settings")
            }

            AccessibleCheckBox {
                text: qsTr("Save these settings as default")
                checked: saveAsDefault
                onCheckedChanged: saveAsDefault = checked
            }
        }
    }

    PostUtils {
        id: postUtils
        skywalker: root.getSkywalker() // qmllint disable missing-type

        onGetPostgateOk: (postgate) => { // qmllint disable signal-handler-parameters
            restrictionDialog.postgate = postgate
            prevAllowQuoting = !postgate.disabledEmbedding
            allowQuoting = prevAllowQuoting
            postgateReceived = true
        }

        onGetPostgateFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }


    function getListUriFromDraft(index) {
        return index < allowListUrisFromDraft.length ? allowListUrisFromDraft[index] : ""
    }

    function saveRestrictionsAsDefault() {
        const allowNobody = restrictReply && !allowMentioned && !allowFollower && !allowFollowing && !hasAllowLists()
        const allowUris = root.getReplyRestrictionListUris(listModelId, allowLists, allowListIndexes)

        postUtils.savePostInteractionSettings(allowMentioned, allowFollower, allowFollowing,
                allowUris, allowNobody, !allowQuoting)
    }

    Component.onCompleted: {
        if (postUri)
            postUtils.getPostgate(postUri)

        if (rootUri) {
            const did = postUtils.extractDidFromUri(rootUri)
            isThreadFromUser = Boolean(did === root.getSkywalker().getUserDid())
        }

        // For a post being composed there are no URIs
        if (!rootUri && !postUri) {
            postgateReceived = true
            isThreadFromUser = true
        }
    }
}
