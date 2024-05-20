import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var chat
    property var skywalker: root.getSkywalker()

    signal closed

    id: conversationsView
    spacing: 0
    clip: true
    model: chat.convoListModel
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    header: SimpleHeader {
        text: qsTr("Conversations")
        onBack: conversationsView.closed()
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        timeline: root.getTimelineView()
        skywalker: conversationsView.skywalker
        messagesActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: positionViewAtBeginning()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: ConvoViewDelegate {
        width: conversationsView.width
    }

    FlickableRefresher {
        inProgress: chat.getConvosInProgress
        topOvershootFun: () => chat.getConvos()
        bottomOvershootFun: () => chat.getConvosNextPage()
        topText: qsTr("Refresh")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.noDirectMessages
        text: qsTr("None")
        list: conversationsView
    }

    GuiSettings {
        id: guiSettings
    }
}
