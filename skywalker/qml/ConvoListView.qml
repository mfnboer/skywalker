import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var chat
    property var skywalker: root.getSkywalker()
    readonly property int margin: 10
    readonly property string sideBarTitle: qsTr("Conversations")

    signal closed

    id: page

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    footer: SkyFooter {
        timeline: root.getTimelineView()
        skywalker: page.skywalker
        messagesActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onMessagesClicked: positionViewAtBeginning()
        onAddConvoClicked: addConvo()
        footerVisible: !root.showSideBar
    }

    SkyTabBar {
        id: tabBar
        y: !root.showSideBar ? 0 : guiSettings.headerMargin
        width: parent.width
        Material.background: guiSettings.backgroundColor
        leftPadding: page.margin
        rightPadding: page.margin

        AccessibleTabButton {
            readonly property int unread: chat.acceptedConvoListModel.unreadCount

            id: tabAccepted
            text: qsTr("Accepted") + (unread > 0 ? "  " : "")
            width: implicitWidth;

            BadgeCounter {
                counter: parent.unread
            }
        }
        AccessibleTabButton {
            readonly property int convoCount: requestView.count
            readonly property int unread: chat.requestConvoListModel.unreadCount

            id: tabRequests
            text: qsTr("Requests") + (convoCount > 0 ? ` (${convoCount})` : "") + (unread > 0 ? "  " : "")
            width: implicitWidth;

            BadgeCounter {
                counter: parent.unread
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

        SkyListView {
            id: acceptedView
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            spacing: 15
            model: chat.acceptedConvoListModel
            clip: true

            delegate: ConvoViewDelegate {
                width: page.width
                onViewConvo: (convo) => page.viewMessages(convo)
                onDeleteConvo: (convo) => page.deleteConvo(convo)
                onMuteConvo: (convo) => chat.muteConvo(convo.id)
                onUnmuteConvo: (convo) => chat.unmuteConvo(convo.id)
                onBlockAuthor: (author) => root.blockAuthor(author)
                onUnblockAuthor: (author) => graphUtils.unblock(author.did, author.viewer.blocking)
            }

            FlickableRefresher {
                inProgress: chat.acceptedConvoListModel.getConvosInProgress
                topOvershootFun: () => chat.getConvos(QEnums.CONVO_STATUS_ACCEPTED)
                bottomOvershootFun: () => chat.getConvosNextPage(QEnums.CONVO_STATUS_ACCEPTED)
                topText: qsTr("Refresh")
            }

            EmptyListIndication {
                y: parent.headerItem ? parent.headerItem.height : 0
                svg: SvgOutline.noDirectMessages
                text: qsTr("None")
                list: acceptedView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: chat.acceptedConvoListModel.getConvosInProgress
            }
        }

        SkyListView {
            id: requestView
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height - page.footer.height
            spacing: 15
            model: chat.requestConvoListModel
            clip: true

            delegate: ConvoViewDelegate {
                width: page.width
                onViewConvo: (convo) => page.viewMessages(convo)
                onDeleteConvo: (convo) => page.deleteConvo(convo)
                onMuteConvo: (convo) => chat.muteConvo(convo.id)
                onUnmuteConvo: (convo) =>chat.unmuteConvo(convo.id)
                onBlockAuthor: (author) => root.blockAuthor(author)
                onUnblockAuthor: (author) => graphUtils.unblock(author.did, author.viewer.blocking)
                onAcceptConvo: (convo) => chat.acceptConvo(convo)
                onBlockAndDeleteConvo: (convo, author) => page.blockAndDeleteConvo(convo, author)
            }

            FlickableRefresher {
                inProgress: chat.requestConvoListModel.getConvosInProgress
                topOvershootFun: () => chat.getConvos(QEnums.CONVO_STATUS_REQUEST)
                bottomOvershootFun: () => chat.getConvosNextPage(QEnums.CONVO_STATUS_REQUEST)
                topText: qsTr("Refresh")
            }

            EmptyListIndication {
                y: parent.headerItem ? parent.headerItem.height : 0
                svg: SvgOutline.noDirectMessages
                text: qsTr("None")
                list: requestView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: chat.requestConvoListModel.getConvosInProgress
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: chat.acceptConvoInProgress
            }
        }
    }

    SvgPlainButton {
        id: moreOptions
        parent: page.header.visible ? page.header : page
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: page.margin
        svg: SvgOutline.moreVert
        accessibleName: qsTr("chat options")
        onClicked: moreMenu.open()

        SkyMenu {
            id: moreMenu
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

    BusyIndicator {
        anchors.centerIn: parent
        running: chat.leaveConvoInProgress
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker

        onBlockOk: (uri) => {
            skywalker.showStatusMessage(qsTr("Blocked"), QEnums.STATUS_LEVEL_INFO)
        }

        onBlockFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onUnblockOk: (uri) => {
            skywalker.showStatusMessage(qsTr("Ublocked"), QEnums.STATUS_LEVEL_INFO)
        }

        onUnblockFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function positionViewAtBeginning() {
        let item = swipeView.currentItem

        if (item)
            item.positionViewAtBeginning()
    }

    function addConvo(msg = "") {
        let component = guiSettings.createComponent("StartConversation.qml")
        let convoPage = component.createObject(page)
        convoPage.onClosed.connect(() => root.popStack())

        convoPage.onSelected.connect((did) => {
            skywalker.chat.startConvoForMember(did, msg)
            root.popStack()
        })

        root.pushStack(convoPage)
    }

    function deleteConvo(convo, parentPage = page, yesCb = () => {}) {
        guiSettings.askYesNoQuestion(parentPage,
                qsTr(`Do you want to delete the conversation with <b>${convo.memberNames}</b>? Your messages will be deleted for you, but not for the other participant.`),
                () => {
                    chat.leaveConvo(convo.id)
                    yesCb()
                })
    }

    function blockAndDeleteConvo(convo, author, parentPage = page, yesCb = () => {}) {
        guiSettings.askYesNoQuestion(parentPage,
                qsTr(`Do you want to block <b>@${author.handle}</b> and delete the conversation?`),
                () => {
                    graphUtils.block(author.did)
                    chat.leaveConvo(convo.id)
                    yesCb()
                })
    }

    function viewMessages(convo) {
        let component = guiSettings.createComponent("MessagesListView.qml")
        let view = component.createObject(page, { chat: chat, convo: convo })
        view.onClosed.connect(() => {
                root.popStack()
            })
        view.onAcceptConvo.connect((convo) => {
                chat.acceptConvo(convo)
                root.popStack();
            })
        view.onDeleteConvo.connect((convo) => {
                page.deleteConvo(convo, view, () => root.popStack())
            })
        view.onBlockAndDeleteConvo.connect((convo, author) => {
                page.blockAndDeleteConvo(convo, author, view, () => root.popStack())
            })
        chat.getMessages(convo.id)
        root.pushStack(view)
    }

    function acceptConvoOkHandler(convo) {
        skywalker.showStatusMessage(qsTr("Conversation accepted"), QEnums.STATUS_LEVEL_INFO)
        viewMessages(convo)
    }

    function leaveConvoOkHandler() {
        skywalker.showStatusMessage(qsTr("Conversation deleted"), QEnums.STATUS_LEVEL_INFO)
    }

    function failureHandler(error) {
        if (root.currentStackIsChat())
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function initHandlers() {
        chat.onAcceptConvoOk.connect(acceptConvoOkHandler)
        chat.onLeaveConvoOk.connect(leaveConvoOkHandler)
        chat.onFailure.connect(failureHandler)
    }

    function destroyHandlers() {
        chat.onAcceptConvoOk.disconnect(acceptConvoOkHandler)
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
