import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    required property var chat
    property var skywalker: root.getSkywalker()

    signal closed

    id: conversationsView
    spacing: 15
    model: chat.convoListModel

    header: SimpleHeader {
        text: qsTr("Conversations")
        onBack: conversationsView.closed()

        SvgPlainButton {
            id: moreOptions
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            svg: SvgOutline.moreVert
            accessibleName: qsTr("chat options")
            onClicked: moreMenu.open()

            Menu {
                id: moreMenu
                modal: true

                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: root.enablePopupShield(false)

                CloseMenuItem {
                    text: qsTr("<b>Options</b>")
                    Accessible.name: qsTr("close options menu")
                }

                AccessibleMenuItem {
                    text: qsTr("Permissions")
                    onTriggered: root.editChatSettings()
                    MenuItemSvg { svg: SvgOutline.key }
                }
            }
        }
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
        onMuteConvo: (convo) => chat.muteConvo(convo.id)
        onUnmuteConvo: (convo) =>chat.unmuteConvo(convo.id)
    }

    FlickableRefresher {
        inProgress: chat.getConvosInProgress
        topOvershootFun: () => chat.getConvos()
        bottomOvershootFun: () => chat.getConvosNextPage()
        topText: qsTr("Refresh")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noDirectMessages
        text: qsTr("None")
        list: conversationsView
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: chat.getConvosInProgress
    }


    function addConvo(msg = "") {
        let component = Qt.createComponent("StartConversation.qml")
        let page = component.createObject(conversationsView)
        page.onClosed.connect(() => root.popStack())

        page.onSelected.connect((did) => {
            skywalker.chat.startConvoForMember(did, msg)
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

    function leaveConvoOkHandler() {
        skywalker.showStatusMessage(qsTr("Conversation deleted"), QEnums.STATUS_LEVEL_INFO)
        chat.getConvos();
    }

    function failureHandler(error) {
        if (root.currentStackIsChat())
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function initHandlers() {
        chat.onLeaveConvoOk.connect(leaveConvoOkHandler)
        chat.onFailure.connect(failureHandler)
    }

    function destroyHandlers() {
        chat.onLeaveConvoOk.disconnect(leaveConvoOkHandler)
        chat.onFailure.disconnect(failureHandler)
    }

    Component.onDestruction: {
        destroyHandlers()
    }

    Component.onCompleted: {
        initHandlers()
    }
}
