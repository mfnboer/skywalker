import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    required property basicprofile author
    required property double postIndexedSecondsAgo
    property contentlabel filteredContentLabel
    readonly property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(author, author.labels, userDid)

    id: postHeader

    // Name eliding seems expensive. Background rendering helps to make the app
    // more responsive.
    Loader {
        width: parent.width
        height: guiSettings.appFontHeight
        active: true
        asynchronous: true

        sourceComponent: RowLayout {
            spacing: 10
            width: parent.width

            AuthorNameAndStatus {
                Layout.fillWidth: true
                userDid: postHeader.userDid
                author: postHeader.author
            }
            DurationLabel {
                id: durationText
                Layout.alignment: Qt.AlignVCenter
                durationSeconds: postIndexedSecondsAgo
                Accessible.ignored: true
                visible: postIndexedSecondsAgo >= 0
            }
        }
    }

    AccessibleText {
        width: parent.width
        bottomPadding: labelsToShow.length > 0 ? 0 : 5
        elide: Text.ElideRight
        text: "@" + author.handle
        font.pointSize: guiSettings.scaledFont(7/8)
        color: guiSettings.handleColor

        Accessible.ignored: true
    }

    Loader {
        active: labelsToShow.length > 0
        width: parent.width
        height: active ? guiSettings.labelHeight + guiSettings.labelRowPadding * 2 : 0
        asynchronous: true
        visible: active

        sourceComponent: Rectangle {
            width: parent.width
            height: parent.height
            color: "transparent"

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.right: undefined
                userDid: postHeader.userDid
                contentLabels: author.labels
                filteredContentLabel: postHeader.filteredContentLabel
                labelsToShow: postHeader.labelsToShow
                contentAuthor: author
            }
        }
    }
}
