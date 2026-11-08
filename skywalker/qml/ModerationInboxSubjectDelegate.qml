import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property Skywalker skywalker: root.getSkywalker()
    required property OzoneUtils ozoneUtils
    required property ozonesubjectview subject
    required property bool endOfList
    readonly property ozoneactionsummary actionSummary: ozoneUtils.getLatestActionSummary(subject)

    signal clicked

    id: view
    height: content.height
    color: subject.isRead ? guiSettings.backgroundColor : guiSettings.postHighLightColor

    MouseArea {
        anchors.fill: parent
        onClicked: view.clicked()
    }

    Column {
        id: content
        x: 10
        width: parent.width
        spacing: 0

        AccessibleText {
            x: 10
            topPadding: 10
            width: parent.width - 20
            font.bold: true
            text: actionSummary.subject
            visible: !subject.latestAction.isNull()
        }

        ContentLabels {
            x: 10
            userDid: skywalker.getUserDid()
            contentAuthor: skywalker.getUser()
            contentLabels: actionSummary.labels
        }

        ModerationPolicySummaryList {
            x: 10
            width: parent.width - 20
            summary: actionSummary.policies
        }

        AccessibleText {
            x: 10
            width: parent.width - 20
            topPadding: 10
            bottomPadding: 10
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: guiSettings.dateTimeIndication(subject.updatedAt)
        }

        Rectangle {
            width: parent.width
            height: 1
            color: guiSettings.separatorColor
        }

        AccessibleText {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            topPadding: 10
            bottomPadding: 50
            text: qsTr("End of list")
            font.italic: true
            visible: endOfList
        }
    }
}