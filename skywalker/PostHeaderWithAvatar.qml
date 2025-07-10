import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Column {
    required property basicprofile author
    required property double postIndexedSecondsAgo
    readonly property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(author.labels)

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
            author: postHeader.author

            onClicked: root.getSkywalker().getDetailedProfile(author.did)
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

        Text {
            Layout.fillWidth: true
            bottomPadding: labelsToShow.length > 0 ? 0 : 5
            elide: Text.ElideRight
            text: "@" + author.handle
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            Accessible.ignored: true
        }
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
                contentLabels: author.labels
                labelsToShow: postHeader.labelsToShow
                contentAuthorDid: author.did
            }
        }
    }
}
