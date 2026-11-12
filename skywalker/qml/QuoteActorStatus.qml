import QtQuick
import QtQuick.Controls
import skywalker

Column {
    property string userDid
    required property actorstatusview authorStatus
    required property basicprofile author

    id: quoteColumn
    padding: 10
    spacing: 5

    PostHeaderWithAvatar {
        width: parent.width - 20
        userDid:quoteColumn.userDid
        author: quoteColumn.author
        showPronouns: true
    }

    AccessibleText {
        width: parent.width - 20
        font.bold: true
        text: authorStatus.externalView.title
    }

    AccessibleText {
        width: parent.width - 20
        wrapMode: Text.Wrap
        maximumLineCount: 5
        text: authorStatus.externalView.description
    }

    AccessibleText {
        id: linkText
        width: parent.width - 10
        text: authorStatus.externalView.uri ? new URL(authorStatus.externalView.uri).hostname : ""
        color: guiSettings.linkColor
    }
}
