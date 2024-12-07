import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property starterpackview starterPack
    property var skywalker: root.getSkywalker()
    readonly property int postFeedModelId: skywalker.createPostFeedModel(starterPack.list)
    readonly property int feedListModelId: skywalker.createFeedListModel()
    readonly property int margin: 10

    signal closed

    id: page

    header: SimpleHeader {
        text: qsTr("Starter pack")
        onBack: page.closed()
    }

    Column {
        id: starterPackHeader
        x: margin
        width: parent.width - 2 * margin

        GridLayout {
            width: parent.width
            columns: 2

            SkyCleanedTextLine {
                topPadding: 10
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: starterPack.name
            }

            SvgButton {
                id: moreButton
                Layout.rowSpan: 2
                svg: SvgOutline.moreVert
                accessibleName: qsTr("more options")
                onClicked: moreMenu.open()

                Menu {
                    id: moreMenu
                    modal: true

                    CloseMenuItem {
                        text: qsTr("<b>Starter pack</b>")
                        Accessible.name: qsTr("close more options menu")
                    }
                    AccessibleMenuItem {
                        text: qsTr("Translate")
                        enabled: starterPack.description
                        onTriggered: root.translateText(starterPack.description)

                        MenuItemSvg { svg: SvgOutline.googleTranslate }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Share")
                        onTriggered: skywalker.shareStarterPack(starterPack)

                        MenuItemSvg { svg: SvgOutline.share }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Report starter pack")
                        onTriggered: root.reportStarterPack(starterPack)

                        MenuItemSvg { svg: SvgOutline.report }
                    }
                }
            }

            AccessibleText {
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: qsTr(`by @${starterPack.creator.handle}`)
            }
        }

        ContentLabels {
            id: contentLabels
            anchors.left: parent.left
            anchors.leftMargin: margin
            anchors.right: undefined
            contentLabels: starterPack.labels
            contentAuthorDid: starterPack.creator.did
        }

        SkyCleanedText {
            topPadding: 10
            width: parent.width
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            textFormat: Text.RichText
            maximumLineCount: 25
            color: guiSettings.textColor
            plainText: starterPack.formattedDescription
            visible: starterPack.description
        }
    }

    TabBar {
        id: feedsBar
        anchors.top: starterPackHeader.bottom
        width: parent.width

        AccessibleTabButton {
            text: qsTr("People")
        }
        AccessibleTabButton {
            text: qsTr("Feeds")
            visible: starterPack.feeds.length > 0
            width: visible ? implicitWidth : 0
        }
        AccessibleTabButton {
            text: qsTr("Posts")
        }
    }

    Rectangle {
        id: feedsSeparator
        anchors.top: feedsBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    StackLayout {
        id: feedStack
        width: parent.width
        anchors.top: feedsSeparator.bottom
        anchors.bottom: parent.bottom
        currentIndex: feedsBar.currentIndex

        AuthorListView {
            id: authorListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: ""
            skywalker: page.skywalker
            modelId: skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LIST_MEMBERS, starterPack.list.uri)
            listUri: starterPack.list.uri
            clip: true

            Component.onCompleted: skywalker.getAuthorList(modelId)
        }

        SkyListView {
            id: feedListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: skywalker.getFeedListModel(feedListModelId)
            boundsBehavior: Flickable.StopAtBounds
            clip: true

            delegate: GeneratorViewDelegate {
                width: feedListView.width
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No feeds")
                list: feedListView
            }
        }

        SkyListView {
            id: postListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: skywalker.getPostFeedModel(postFeedModelId)
            clip: true

            delegate: PostFeedViewDelegate {
                width: postListView.width
            }

            StackLayout.onIsCurrentItemChanged: {
                if (!StackLayout.isCurrentItem)
                    cover()
            }

            FlickableRefresher {
                inProgress: skywalker.getFeedInProgress
                topOvershootFun: () => skywalker.getListFeed(postFeedModelId)
                bottomOvershootFun: () => skywalker.getListFeedNextPage(postFeedModelIds)
                topText: qsTr("Pull down to refresh feed")
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("Feed is empty")
                list: postListView
            }

            BusyIndicator {
                id: busyIndicator
                anchors.centerIn: parent
                running: skywalker.getFeedInProgress
            }
        }
    }


    Component.onDestruction: {
        skywalker.removePostFeedModel(postFeedModelId)
        skywalker.removeFeedListModel(feedListModelId)
    }

    Component.onCompleted: {
        skywalker.getListFeed(postFeedModelId)

        let feedListModel = skywalker.getFeedListModel(feedListModelId)
        feedListModel.addFeeds(starterPack.feeds)
    }
}
