import QtQuick
import QtQuick.Controls
import skywalker

Column {
    property string userDid
    required property convoview convo

    id: quoteColumn
    padding: 10
    spacing: 5

    AccessibleText {
        width: parent.width - 20
        font.bold: true
        text: convo.kind === QEnums.CONVO_KIND_DIRECT ? qsTr("Chat with") : qsTr("Group chat")
    }

    AccessibleText {
        width: parent.width - 20
        text: convo.title
        visible: convo.kind === QEnums.CONVO_KIND_GROUP
    }

    PostHeaderWithAvatar {
        width: parent.width - 20
        userDid: quoteColumn.userDid
        author: convo.getOwner().basicProfile
        showPronouns: true
    }
}
