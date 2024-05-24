import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var chat
    property var skywalker: root.getSkywalker()

    signal closed

    id: conversationsView
    spacing: 10
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
        onAddConvoClicked: addConvo()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: ConvoViewDelegate {
        width: conversationsView.width
        onViewConvo: (convo) => conversationsView.viewMessages(convo)
        onDeleteConvo: (convo) => conversationsView.deleteConvo(convo)
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

    BusyIndicator {
        anchors.centerIn: parent
        running: chat.getConvosInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    function addConvo() {
        let component = Qt.createComponent("StartConversation.qml")
        let page = component.createObject(conversationsView)
        page.onClosed.connect(() => root.popStack())

        page.onSelected.connect((did) => {
            skywalker.chat.startConvoForMember(did)
            root.popStack()
        })

        root.pushStack(page)
    }

    function deleteConvo(convo) {
        guiSettings.askYesNoQuestion(conversationsView,
                qsTr(`Do you want to delete the conversation with <b>${convo.memberNames}</b>. Your messages will be deleted for you, but not for the other participant.`),
                () => chat.leaveConvo(convo.id))
    }

    function viewMessages(convo) {
        let component = Qt.createComponent("MessagesListView.qml")
        let view = component.createObject(conversationsView, { chat: chat, convo: convo })
        view.onClosed.connect(() => root.popStack())
        chat.getMessages(convo.id)
        root.pushStack(view)
    }

    function getConvosFailedHandler(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function leaveConvoFailedHandler(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function initHandlers() {
        chat.onGetConvosFailed.connect(getConvosFailedHandler)
        chat.onLeaveConvoFailed.connect(leaveConvoFailedHandler)
    }

    function destroyHandlers() {
        chat.onGetConvosFailed.disconnect(getConvosFailedHandler)
        chat.onLeaveConvoFailed.disconnect(leaveConvoFailedHandler)
    }

    Component.onDestruction: {
        destroyHandlers()
    }

    Component.onCompleted: {
        initHandlers()
    }
}
