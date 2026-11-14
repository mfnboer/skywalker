import QtQuick
import QtQuick.Controls
import skywalker

Column {
    property string userDid
    required property profilelinkview link
    required property basicprofile author
    property string backgroundColor: guiSettings.backgroundColor

    id: quoteColumn
    padding: 10
    spacing: 5

    PostHeaderWithAvatar {
        width: parent.width - 20
        userDid: quoteColumn.userDid
        author: quoteColumn.author
        showPronouns: true
    }

    ProfileLinkRow {
        x: 10
        width: parent.width - 20
        backgroundColor: quoteColumn.backgroundColor
        link: quoteColumn.link
    }
}
