import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
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

        SkyCleanedText {
            topPadding: 10
            width: parent.width
            elide: Text.ElideRight
            font.bold: true
            color: guiSettings.textColor
            plainText: starterPack.name
        }

        AccessibleText {
            width: parent.width
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: qsTr(`by @${starterPack.creator.handle}`)
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

            delegate: GeneratorViewDelegate {
                width: feedListView.width
            }

            EmptyListIndication {
                svg: svgOutline.noPosts
                text: qsTr("No feeds")
                list: feedListView
            }
        }

        SkyListView {
            id: postListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: skywalker.getPostFeedModel(postFeedModelId)

            delegate: PostFeedViewDelegate {
                width: postListView.width
            }

            FlickableRefresher {
                inProgress: skywalker.getFeedInProgress
                topOvershootFun: () => skywalker.getListFeed(postFeedModelId)
                bottomOvershootFun: () => skywalker.getListFeedNextPage(postFeedModelIds)
                topText: qsTr("Pull down to refresh feed")
            }

            EmptyListIndication {
                svg: svgOutline.noPosts
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

    GuiSettings {
        id: guiSettings
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
