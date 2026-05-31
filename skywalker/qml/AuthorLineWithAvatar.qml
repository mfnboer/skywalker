import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    required property basicprofile author
    property contentlabel filteredContentLabel
    readonly property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(author, author.labels, userDid)

    id: postHeader

    RowLayout {
        width: parent.width
        spacing: 10

        Avatar {
            id: avatar
            Layout.preferredWidth: Math.max(guiSettings.appFontHeight, 24)
            userDid: postHeader.userDid
            author: postHeader.author

            onClicked: root.getSkywalker(userDid).getDetailedProfile(author.did)
        }

        // Name eliding seems expensive. Background rendering helps to make the app
        // more responsive.
        Loader {
            Layout.fillWidth: true
            height: guiSettings.appFontHeight
            active: true
            asynchronous: true

            sourceComponent: Item {
                AuthorNameAndStatus {
                    id: authorName
                    width: parent.width
                    userDid: postHeader.userDid
                    author: postHeader.author
                }

                AccessibleText {
                    x: authorName.advanceWidth + 10
                    anchors.bottom: authorName.bottom
                    width: parent.width - x
                    elide: Text.ElideRight
                    font.pointSize: guiSettings.scaledFont(7/8)
                    color: guiSettings.handleColor
                    text: "@" + author.handle
                }
            }
        }
    }
}
