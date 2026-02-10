import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    required property basicprofile author
    required property double postIndexedSecondsAgo
    property bool showPronouns
    property contentlabel filteredContentLabel
    readonly property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(author, author.labels, userDid)

    id: postHeader

    GridLayout {
        width: parent.width
        columns: 2
        columnSpacing: 10
        rowSpacing: 0

        Avatar {
            id: avatar
            Layout.rowSpan: 2
            Layout.topMargin: 3
            Layout.preferredWidth: 34
            Layout.alignment: Qt.AlignTop
            userDid: postHeader.userDid
            author: postHeader.author

            onClicked: root.getSkywalker(userDid).getDetailedProfile(author.did)
        }

        // Name eliding seems expensive. Background rendering helps to make the app
        // more responsive.
        Loader {
            Layout.fillWidth: true
            Layout.preferredHeight: guiSettings.appFontHeight
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
            readonly property bool displayPronouns: showPronouns && author.pronouns

            Layout.fillWidth: true
            bottomPadding: labelsToShow.length > 0 ? 0 : 5
            elide: displayPronouns ? Text.ElideMiddle : Text.ElideRight
            text: "@" + author.handle + (displayPronouns ? ` (${author.pronouns})` : "")
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            Accessible.ignored: true
        }
    }

    Loader {
        active: labelsToShow.length > 0
        width: parent.width
        visible: active

        sourceComponent: Rectangle {
            width: parent.width
            height: contentLabels.height + guiSettings.labelRowPadding
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
