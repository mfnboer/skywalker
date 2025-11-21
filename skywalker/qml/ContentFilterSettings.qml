import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property ContentGroupListModel globalLabelModel
    required property int labelerAuthorListModelId
    property Skywalker skywalker: root.getSkywalker()
    property ContentFilter contentFilter: skywalker.getContentFilter()
    property int margin: 10
    readonly property string sideBarTitle: qsTr("Content Filtering")
    readonly property SvgImage sideBarSvg: SvgOutline.visibility
    readonly property SvgImage sideBarButtonSvg: SvgOutline.moreVert
    readonly property string sideBarButtonName: qsTr("options")
    property list<string> listPrefUris: []

    signal closed()

    id: page
    topPadding: 10
    bottomPadding: 10

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()

        SvgPlainButton {
            anchors.right: parent.right
            anchors.top: parent.top
            svg: sideBarButtonSvg
            accessibleName: sideBarButtonName
            onClicked: sideBarButtonClicked()

            SkyMenu {
                id: moreMenu
                width: 250

                CloseMenuItem {
                    text: qsTr("<b>Options</b>")
                    Accessible.name: qsTr("close options menu")
                }

                AccessibleMenuItem {
                    id: followingItem
                    text: qsTr("Add filters for following")
                    enabled: !contentFilter.hasFollowingPrefs
                    onTriggered: addFollowingPrefs()
                    MenuItemSvg { svg: SvgOutline.group }
                }

                AccessibleMenuItem {
                    text: qsTr("Add filters for list")
                    onTriggered: selectList()
                    MenuItemSvg { svg: SvgOutline.list }
                }

                AccessibleMenuItem {
                    text: qsTr("Help")
                    onTriggered: showHelp()
                    MenuItemSvg { svg: SvgOutline.help }
                }
            }
        }
    }

    SkyTabBar {
        id: tabBar
        width: parent.width
        clip: true
        visible: listPrefUris.length > 0

        AccessibleTabButton {
            id: tabAll
            text: qsTr("All content")
        }

        Repeater {
            model: listPrefUris

            SkyTabListButton {
                required property string modelData
                readonly property alias listUri: listPrefTab.modelData

                id: listPrefTab
                list: contentFilter.getList(listUri)
                name: contentFilter.getListName(listUri)

                onClosed: closeTab(listUri)
            }
        }
    }

    Rectangle {
        id: tabSeparator
        anchors.top: tabBar.visible ? tabBar.bottom : tabBar.top
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    SwipeView {
        property var prevItem: allContentItem

        id: swipeView
        anchors.top: tabSeparator.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: tabBar.currentIndex

        onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)
        onCurrentItemChanged: {
            if (prevItem)
                prevItem.saveModel()

            prevItem = currentItem
        }

        // All content
        Flickable {
            id: allContentItem
            clip: true
            contentHeight: labelerListView.y + labelerListView.height
            flickableDirection: Flickable.VerticalFlick
            boundsBehavior: Flickable.StopAtBounds

            HeaderText {
                id: globalContentFilters
                text: qsTr("Global filters")
            }

            AccessibleCheckBox {
                anchors.top: globalContentFilters.bottom
                anchors.topMargin: 10
                bottomPadding: 20
                id: adultContentSwitch
                width: parent.width
                Material.accent: guiSettings.buttonColor
                text: qsTr("Adult content")
                checked: page.globalLabelModel.adultContent
                onCheckedChanged: page.globalLabelModel.adultContent = checked

                Accessible.role: Accessible.Button
                Accessible.name: text
                Accessible.onPressAction: toggle()
            }

            ListView {
                id: globalLabelListView
                anchors.top: adultContentSwitch.bottom
                width: parent.width
                height: contentHeight
                clip: true
                model: page.globalLabelModel
                boundsBehavior: Flickable.StopAtBounds

                Accessible.role: Accessible.List

                delegate: ContentGroupDelegate {
                    width: parent.width
                    isSubscribed: true
                    adultContent: page.globalLabelModel.adultContent
                    isListPref: false
                }
            }

            HeaderText {
                id: subscribedLabelers
                anchors.top: globalLabelListView.bottom
                text: qsTr("Subscribed labeler filters")
            }

            ListView {
                id: labelerListView
                anchors.top: subscribedLabelers.bottom
                anchors.topMargin: 10
                width: parent.width
                height: contentHeight
                clip: true
                model: skywalker.getAuthorListModel(page.labelerAuthorListModelId)
                boundsBehavior: Flickable.StopAtBounds

                Accessible.role: Accessible.List

                delegate: AuthorViewDelegate {
                    id: labelerDelegate
                    width: parent.width
                    textRightPadding: 20
                    highlight: contentFilter.hasNewLabels(author.did)
                    maximumDescriptionLineCount: 3
                    formatDescription: false

                    SkySvg {
                        height: 40
                        width: height
                        x: parent.width - 40
                        y: (parent.height + height) / 2
                        svg: SvgOutline.navigateNext
                        color: guiSettings.textColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            skywalker.saveGlobalContentFilterPreferences();
                            skywalker.getDetailedProfile(author.did)
                            labelerDelegate.highlight = false
                        }
                    }
                }
            }

            function saveModel() {
                skywalker.saveGlobalContentFilterPreferences()
            }
        }

        Repeater {
            id: listViews
            model: listPrefUris

            Flickable {
                required property string modelData
                readonly property alias listUri: labelPrefsList.modelData

                id: labelPrefsList
                clip: true
                contentHeight: contentItem.childrenRect.height
                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.StopAtBounds

                HeaderText {
                    id: globalHeaderList
                    text: qsTr("Global filters")
                }

                ListView {
                    readonly property int modelId: skywalker.createGlobalContentGroupListModel(listUri)

                    id: globalLabelListViewList
                    anchors.top: globalHeaderList.bottom
                    width: parent.width
                    height: contentHeight
                    clip: true
                    model: skywalker.getContentGroupListModel(modelId)
                    boundsBehavior: Flickable.StopAtBounds

                    Accessible.role: Accessible.List

                    delegate: ContentGroupDelegate {
                        width: parent.width
                        isSubscribed: true
                        adultContent: page.globalLabelModel.adultContent
                        isListPref: true
                    }
                }

                HeaderText {
                    id: subscribedLabelersList
                    anchors.top: globalLabelListViewList.bottom
                    text: qsTr("Subscribed labeler filters")
                }

                ListView {
                    id: labelerListViewList
                    anchors.top: subscribedLabelersList.bottom
                    anchors.topMargin: 10
                    width: parent.width
                    height: contentHeight
                    clip: true
                    model: skywalker.getAuthorListModel(page.labelerAuthorListModelId)
                    boundsBehavior: Flickable.StopAtBounds

                    Accessible.role: Accessible.List

                    delegate: AuthorViewDelegate {
                        id: labelerListDelegate
                        width: parent.width
                        textRightPadding: 20
                        maximumDescriptionLineCount: 3
                        formatDescription: false
                        highlight: contentFilter.hasListPref(labelPrefsList.listUri, author.did)
                        highlightColor: guiSettings.labelPrefDefaultColor

                        SkySvg {
                            height: 40
                            width: height
                            x: parent.width - 40
                            y: (parent.height + height) / 2
                            svg: SvgOutline.navigateNext
                            color: guiSettings.textColor
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                labelPrefsList.saveModel()
                                skywalker.getDetailedProfile(author.did, labelPrefsList.listUri)
                            }
                        }

                        function refresh(labelerDid) {
                            if (author.did === labelerDid)
                                labelerListDelegate.highlight = contentFilter.hasListPref(labelPrefsList.listUri, author.did)
                        }
                    }
                }

                function refresh(labelerDid) {
                    for (let i = 0; i < labelerListViewList.count; ++i) {
                        const item = labelerListViewList.itemAtIndex(i)

                        if (item)
                            item.refresh(labelerDid)
                    }
                }

                function saveModel() {
                    skywalker.saveContentFilterPreferences(globalLabelListViewList.model)
                }

                function removeModel() {
                    skywalker.removeContentGroupListModel(globalLabelListViewList.modelId)
                }
            }
        }
    }

    Utils {
        id: utils
        skywalker: page.skywalker
    }

    function selectList() {
        const modelId = skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_CURATE, skywalker.getUserDid())
        skywalker.getListList(modelId)
        let component = guiSettings.createComponent("SelectList.qml")
        let dialog = component.createObject(page, { listModelId: modelId })
        dialog.onAccepted.connect(() => {
            const list = dialog.getList()
            console.debug("Selected:", list.uri, list.name)

            if (!list.isNull())
                addListPrefs(list)

            dialog.destroy()
            skywalker.removeListListModel(modelId)
        })
        dialog.onNewList.connect(() => {
            const model = skywalker.getListListModel(modelId)
            dialog.close()
            root.newList(model, QEnums.LIST_PURPOSE_CURATE, skywalker.getUserDid(), () => { dialog.open() })
        })
        dialog.onRejected.connect(() => {
            dialog.destroy()
            skywalker.removeListListModel(modelId)
        })
        dialog.open()
    }

    function addFollowingPrefs() {
        contentFilter.createFollowingPrefs()
        const followingUri = utils.getFollowingUri()
        listPrefUris.unshift(followingUri)
    }

    function removeFollowingPrefs() {
        removeListPrefUri(utils.getFollowingUri())
        contentFilter.removeFollowing()
    }

    function addListPrefs(list) {
        if (listPrefUris.indexOf(list.uri) >= 0 ) {
            skywalker.showStatusMessage(qsTr(`${list.name} already added`), QEnums.STATUS_LEVEL_INFO)
            return
        }

        contentFilter.createListPref(list)
        listPrefUris.push(list.uri)
    }

    function removeListPrefs(listUri) {
        if (utils.isFollowingListUri(listUri)) {
            removeFollowingPrefs()
        } else {
            removeListPrefUri(listUri)
            contentFilter.removeList(listUri)
        }
    }

    function closeTab(listUri) {
        const listName = contentFilter.getListName(listUri)

        guiSettings.askYesNoQuestion(page,
            qsTr(`Do you want to remove your label preferences for ${listName}?`),
            () => removeListPrefs(listUri)
        )
    }

    function handleAddListFailure(listUri, error) {
        skywalker.showStatusMessage(qsTr(`Failed to add list: ${error}`), QEnums.STATUS_LEVEL_ERROR)
        removeListPrefUri(listUri)
    }

    function removeListPrefUri(listUri) {
        const index = listPrefUris.indexOf(listUri)

        if (index >= 0)
            listPrefUris.splice(index, 1)
    }

    function sideBarButtonClicked() {
        moreMenu.open()
    }

    function reloadSubscribedLabelers() {
        skywalker.getAuthorList(labelerAuthorListModelId)
    }

    function refreshLabelers(listUri, labelerDid) {
        for (let i = 0; i < page.listPrefUris.length; ++i) {
            if (listPrefUris[i] !== listUri)
                continue

            let item = listViews.itemAt(i)

            if (item)
                item.refresh(labelerDid)

            break
        }
    }

    function showHelp() {
        guiSettings.notice(page, qsTr(
            "Here you can set the your preferences for labeled content." +
            "<ul>" +
            "<li><b>show</b> - show content</li>" +
            "<li><b>warn</b> - warn for content before showing</li>" +
            "<li><b>hide</b> - remove content from your feeds</li>" +
            "</ul>" +
            "You can override your preferences set for <i>all content</i> for users you follow or users in a list. " +
            "The preferences from <i>all content</i> will be highlighted, so you can see where you made changes.<br><br>" +
            "The subscribed labelers are the same for all tabs. You cannot unsubscribe from a labeler for a list only.<br><br>" +
            "For example, you can set strict preferences for <i>all content</i> and set less strict preferences for your friends.<br><br>" +
            "NOTE: your preferences for <i>all content</i> are stored in the network and shared with other apps. Your preferences for users " +
            "you follow and lists are stored locally on your device."
        ))
    }

    Component.onDestruction: {
        contentFilter.onSubscribedLabelersChanged.disconnect(reloadSubscribedLabelers)
        contentFilter.onListPrefsChanged.disconnect(refreshLabelers)
        contentFilter.onListAddingFailed.disconnect(handleAddListFailure)

        skywalker.saveGlobalContentFilterPreferences()

        for (let i = 0; i < listPrefUris.length; ++i) {
            let item = listViews.itemAt(i)

            if (item) {
                item.saveModel()
                item.removeModel()
            }
        }

        skywalker.removeAuthorListModel(labelerAuthorListModelId)
    }

    Component.onCompleted: {
        listPrefUris = contentFilter.getListUris()

        contentFilter.onSubscribedLabelersChanged.connect(reloadSubscribedLabelers)
        contentFilter.onListPrefsChanged.connect(refreshLabelers)
        contentFilter.onListAddingFailed.connect(handleAddListFailure)
    }
}
