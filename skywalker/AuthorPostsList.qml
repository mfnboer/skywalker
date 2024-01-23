import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property detailedprofile author
    required property var enclosingView
    required property var getFeed
    required property var getFeedNextPage
    required property var getEmptyListIndicationSvg
    required property var getEmptyListIndicationText
    required property var visibilityShowProfileLink
    required property var disableWarning
    property int modelId: -1
    property int feedFilter: QEnums.AUTHOR_FEED_FILTER_POSTS

    id: authorPostsList
    width: parent.width
    height: parent.height
    clip: true
    spacing: 0
    model: modelId >= 0 ? skywalker.getAuthorFeedModel(page.modelId) : null
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}
    interactive: !enclosingView.interactive

    StackLayout.onIsCurrentItemChanged: {
        if (StackLayout.isCurrentItem && modelId < 0) {
            modelId = skywalker.createAuthorFeedModel(author, feedFilter)
            model = skywalker.getAuthorFeedModel(modelId)
            getFeed(modelId)
        }
    }

    onContentYChanged: {
        if (contentY < 0) {
            contentY = 0
            enclosingView.interactive = true
        }
    }

    delegate: PostFeedViewDelegate {
        viewWidth: enclosingView.width
    }

    FlickableRefresher {
        inProgress: skywalker.getAuthorFeedInProgress
        verticalOvershoot: authorPostsList.verticalOvershoot
        //topOvershootFun: () => getFeed(modelId)
        bottomOvershootFun: () => getFeedNextPage(modelId)
        topText: ""
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getAuthorFeedInProgress
    }

    EmptyListIndication {
        id: noPostIndication
        svg: getEmptyListIndicationSvg()
        text: getEmptyListIndicationText()
        list: authorPostsList
        onLinkActivated: (link) => root.viewListByUri(link, false)
    }
    Text {
        anchors.top: noPostIndication.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        elide: Text.ElideRight
        textFormat: Text.RichText
        text: `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show profile") + "</a>"
        visible: visibilityShowProfileLink()
        onLinkActivated: disableWarning(modelId)
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        if (modelId >= 0)
            skywalker.removeAuthorFeedModel(modelId)
    }
}
