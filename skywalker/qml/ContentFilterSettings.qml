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

                CloseMenuItem {
                    text: qsTr("<b>Options</b>")
                    Accessible.name: qsTr("close options menu")
                }

                AccessibleMenuItem {
                    text: qsTr("Add following filters")
                    enabled: !contentFilter.hasFollowingPrefs
                    onTriggered: addFollowingPrefs()
                }
            }
        }
    }

    SkyTabBar {
        id: tabBar
        width: parent.width
        clip: true

        AccessibleTabButton {
            id: tabAll
            text: qsTr("All content")
        }

        Repeater {
            model: listPrefUris

            AccessibleTabButton {
                required property string modelData
                readonly property alias listUri: listPrefTab.modelData

                id: listPrefTab
                text: contentFilter.getListName(listUri)
            }
        }
    }

    Rectangle {
        id: tabSeparator
        anchors.top: tabBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    SwipeView {
        id: swipeView
        anchors.top: tabSeparator.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: tabBar.currentIndex

        onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)
        onCurrentItemChanged: currentItem.saveModel()

        // All content
        Flickable {
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

    function addFollowingPrefs() {
        contentFilter.createFollowingPrefs()
        listPrefUris.unshift("following")
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

    Component.onDestruction: {
        contentFilter.onSubscribedLabelersChanged.disconnect(reloadSubscribedLabelers)
        contentFilter.onListPrefsChanged.disconnect(refreshLabelers)

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
        if (contentFilter.hasFollowingPrefs)
            listPrefUris.unshift("following")

        contentFilter.onSubscribedLabelersChanged.connect(reloadSubscribedLabelers)
        contentFilter.onListPrefsChanged.connect(refreshLabelers)
    }
}
