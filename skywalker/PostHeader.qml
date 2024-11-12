import QtQuick
import QtQuick.Controls.Material
import skywalker

Column {
    required property basicprofile author
    required property double postIndexedSecondsAgo
    readonly property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(author.labels)

    id: postHeader

    // Name eliding seems expensive. Background rendering helps to make the app
    // more responsive.
    Loader {
        width: parent.width
        height: guiSettings.appFontHeight
        active: true
        asynchronous: true

        sourceComponent: Row {
            spacing: 10
            width: parent.width

            SkyCleanedTextLine {
                width: parent.width - durationText.width - parent.spacing
                elide: Text.ElideRight
                plainText: author.name
                font.bold: true
                color: guiSettings.textColor

                Accessible.ignored: true
            }
            Text {
                id: durationText
                text: guiSettings.durationToString(postIndexedSecondsAgo)
                font.pointSize: guiSettings.scaledFont(7/8)
                color: Material.color(Material.Grey)

                Accessible.ignored: true
            }
        }
    }

    Text {
        width: parent.width
        bottomPadding: 5
        elide: Text.ElideRight
        text: "@" + author.handle
        font.pointSize: guiSettings.scaledFont(7/8)
        color: guiSettings.handleColor

        Accessible.ignored: true
    }

    Loader {
        active: labelsToShow.length > 0
        width: parent.width
        height: active ? guiSettings.labelHeight + guiSettings.labelRowPadding : 0
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
