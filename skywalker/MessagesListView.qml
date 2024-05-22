import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var chat
    required property convoview convo
    property basicprofile firstMember: convo.members.length > 0 ? convo.members[0].basicProfile : skywalker.getUserProfile()
    property var skywalker: root.getSkywalker()
    readonly property int margin: 10

    signal closed

    id: messagesView
    spacing: 0
    clip: true
    model: chat.getMessageListModel(convo.id)
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    header: MessagesListHeader {
        convo: messagesView.convo
        onBack: messagesView.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: MessageViewDelegate {
        viewWidth: messagesView.width
    }

    FlickableRefresher {
        inProgress: chat.getMessagesInProgress
        topOvershootFun: () => chat.getMessagesNextPage(convo.id)
        bottomOvershootFun: () => chat.getMessages(convo.id)
        topText: qsTr("Get older messages")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.noDirectMessages
        text: qsTr("None")
        list: messagesView
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: chat.getMessagesInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        positionViewAtEnd()
    }
}
