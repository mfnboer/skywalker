import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property listview list
    property bool isSavedList: skywalker.favoriteFeeds.isSavedFeed(list.uri)
    property bool isPinnedList: skywalker.favoriteFeeds.isPinnedFeed(list.uri)

    signal closed

    id: page

    header: SimpleHeader {
        text: guiSettings.listTypeName(list.purpose)
        onBack: closed()
    }

    GridLayout {
        id: grid
        rowSpacing: 0
        columns: 3
        x: 10
        width: parent.width - 20

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 10
            color: "transparent"
        }

        FeedAvatar {
            x: 8
            y: 5
            width: 100
            avatarUrl: list.avatar
            onClicked: {
                if (list.avatar)
                    root.viewFullImage([list.imageView], 0)
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            leftPadding: 10
            rightPadding: 10

            Text {
                width: parent.width
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                font.bold: true
                font.pointSize: guiSettings.scaledFont(16/8)
                color: guiSettings.textColor
                text: list.name
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.listTypeName(list.purpose)
            }

            Text {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: list.creator.name

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(list.creator.did)
                }
            }

            Text {
                topPadding: 2
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + list.creator.handle


                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(list.creator.did)
                }
            }
        }

        Column {
            SvgButton {
                id: moreButton
                svg: svgOutline.moreVert

                onClicked: {
                    switch (list.purpose) {
                    case QEnums.LIST_PURPOSE_CURATE:
                        if (list.creator.did === skywalker.getUserDid())
                            moreMenuOwnUserList.popup(moreButton)
                        else
                            moreMenuOtherUserList.popup(moreButton)

                        break
                    case QEnums.LIST_PURPOSE_MOD:
                        moreMenuModList.popup(moreButton)
                        break
                    }
                }
            }

            SvgButton {
                id: addUser
                svg: svgOutline.addUser
                visible: isOwnList()
                onClicked: page.addUser()
            }
        }

        Text {
            topPadding: 5
            bottomPadding: 10
            Layout.columnSpan: 3
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            maximumLineCount: 1000
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: list.formattedDescription

            onLinkActivated: (link) => root.openLink(link)
        }
    }

    AuthorListView {
        id: authorListView
        width: parent.width
        anchors.top: grid.bottom
        anchors.bottom: parent.bottom
        title: ""
        skywalker: page.skywalker
        modelId: skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LIST_MEMBERS, list.uri)
        allowDeleteItem: isOwnList()
        clip: true

        Component.onCompleted: skywalker.getAuthorList(modelId)
    }

    Menu {
        id: moreMenuOwnUserList

        MenuItem {
            text: isPinnedList ? qsTr("Remove favorite") : qsTr("Add favorite")
            onTriggered: {
                if (isPinnedList)
                    skywalker.favoriteFeeds.removeList(list) // We never show own lists as saved
                else
                    skywalker.favoriteFeeds.pinList(list, true)

                isPinnedList = !isPinnedList
                isSavedList = skywalker.favoriteFeeds.isSavedFeed(list.uri)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg {
                svg: isPinnedList ? svgFilled.star : svgOutline.star
                color: isPinnedList ? guiSettings.favoriteColor : guiSettings.textColor
            }
        }
        MenuItem {
            text: qsTr("Translate")
            onTriggered: root.translateText(list.description)

            MenuItemSvg { svg: svgOutline.googleTranslate }
        }
        MenuItem {
            text: qsTr("Share")
            onTriggered: skywalker.shareList(list)

            MenuItemSvg { svg: svgOutline.share }
        }
        MenuItem {
            text: qsTr("Report list")
            onTriggered: root.reportList(list)

            MenuItemSvg { svg: svgOutline.report }
        }
    }

    Menu {
        id: moreMenuOtherUserList

        MenuItem {
            text: isSavedList ? qsTr("Unsave list") : qsTr("Save list")
            onTriggered: {
                if (isSavedList)
                    skywalker.favoriteFeeds.removeList(list)
                else
                    skywalker.favoriteFeeds.addList(list)

                isSavedList = !isSavedList
                isPinnedList = skywalker.favoriteFeeds.isPinnedFeed(list.uri)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg { svg: isSavedList ? svgOutline.remove : svgOutline.add }
        }
        MenuItem {
            text: isPinnedList ? qsTr("Remove favorite") : qsTr("Add favorite")
            onTriggered: {
                skywalker.favoriteFeeds.pinList(list, !isPinnedList)
                isPinnedList = !isPinnedList
                isSavedList = skywalker.favoriteFeeds.isSavedFeed(list.uri)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg {
                svg: isPinnedList ? svgFilled.star : svgOutline.star
                color: isPinnedList ? guiSettings.favoriteColor : guiSettings.textColor
            }
        }
        MenuItem {
            text: qsTr("Translate")
            onTriggered: root.translateText(list.description)

            MenuItemSvg { svg: svgOutline.googleTranslate }
        }
        MenuItem {
            text: qsTr("Share")
            onTriggered: skywalker.shareList(list)

            MenuItemSvg { svg: svgOutline.share }
        }
        MenuItem {
            text: qsTr("Report list")
            onTriggered: root.reportList(list)

            MenuItemSvg { svg: svgOutline.report }
        }
    }

    Menu {
        id: moreMenuModList

        MenuItem {
            text: qsTr("Translate")
            onTriggered: root.translateText(list.description)

            MenuItemSvg { svg: svgOutline.googleTranslate }
        }
        MenuItem {
            text: qsTr("Share")
            onTriggered: skywalker.shareList(list)

            MenuItemSvg { svg: svgOutline.share }
        }
        MenuItem {
            text: qsTr("Report list")
            onTriggered: root.reportList(list)

            MenuItemSvg { svg: svgOutline.report }
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker

        onAddListUserOk: (did, itemUri, itemCid) => profileUtils.getProfileView(did, itemUri)
        onAddListUserFailed: (error) => statusPopup.show(qsTr(`Failed to add user: ${error}`), QEnums.STATUS_LEVEL_ERROR)
    }

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker

        onProfileViewOk: (profile, listItemUri) => authorListView.model.prependAuthor(profile, listItemUri)
        onProfileViewFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    GuiSettings {
        id: guiSettings
    }

    function addUser() {
        let component = Qt.createComponent("SearchAuthor.qml")
        let searchPage = component.createObject(page, { skywalker: skywalker })
        searchPage.onAuthorClicked.connect((profile) => {
            graphUtils.addListUser(list.uri, profile.did)
            root.popStack()
        })
        searchPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(searchPage)
    }

    function isOwnList() {
        return skywalker.getUserDid() === list.creator.did
    }
}
