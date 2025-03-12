import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var chat
    property var skywalker: root.getSkywalker()
    readonly property int margin: 10

    signal closed

    id: page

    header: SimpleHeader {
        text: qsTr("Conversations")
        onBack: page.closed()

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

    footer: SkyFooter {
        timeline: root.getTimelineView()
        skywalker: page.skywalker
        messagesActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: positionViewAtBeginning()
        onAddConvoClicked: addConvo()
    }

    SkyTabBar {
        id: tabBar
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
                onUnmuteConvo: (convo) =>chat.unmuteConvo(convo.id)
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
        }
    }

    function addConvo(msg = "") {
        let component = Qt.createComponent("StartConversation.qml")
        let convoPage = component.createObject(page)
        convoPage.onClosed.connect(() => root.popStack())

        convoPage.onSelected.connect((did) => {
            skywalker.chat.startConvoForMember(did, msg)
            root.popStack()
        })

        root.pushStack(convoPage)
    }

    function deleteConvo(convo) {
        guiSettings.askYesNoQuestion(page,
                qsTr(`Do you want to delete the conversation with <b>${convo.memberNames}</b>. Your messages will be deleted for you, but not for the other participant.`),
                () => chat.leaveConvo(convo.id))
    }

    function viewMessages(convo) {
        let component = Qt.createComponent("MessagesListView.qml")
        let view = component.createObject(page, { chat: chat, convo: convo })
        view.onClosed.connect(() => root.popStack())
        chat.getMessages(convo.id)
        root.pushStack(view)
    }

    function leaveConvoOkHandler() {
        skywalker.showStatusMessage(qsTr("Conversation deleted"), QEnums.STATUS_LEVEL_INFO)
        chat.getConvos(QEnums.CONVO_STATUS_ACCEPTED);
        chat.getConvos(QEnums.CONVO_STATUS_REQUEST);
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
