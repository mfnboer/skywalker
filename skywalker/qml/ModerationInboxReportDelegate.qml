import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property OzoneUtils ozoneUtils
    required property ozonereportview report
    required property basicprofile author
    required property string authorError
    required property bool endOfList

    signal clicked

    id: view
    height: content.height
    color: report.isRead ? guiSettings.backgroundColor : guiSettings.postHighLightColor

    Column {
        id: content
        x: 10
        width: parent.width
        spacing: 0

        AccessibleText {
            x: 10
            topPadding: 10
            width: parent.width - 50
            font.bold: true
            text: ozoneUtils.subjectTypeToString(report)
        }

        PostHeaderWithAvatar {
            x: 10
            topPadding: 10
            width: parent.width - 50
            userDid: skywalker.getUserDid()
            author: view.author
            visible: !author.isNull()
        }

        AccessibleText {
            x: 10
            topPadding: 10
            width: parent.width - 50
            font.bold: true
            text: authorError
            visible: author.isNull()
        }

        AccessibleText {
            x: 10
            topPadding: 10
            width: parent.width - 50
            text: report.reasonType
            visible: report.reasonType
        }

        AccessibleText {
            x: 10
            width: parent.width - 50
            topPadding: report.reasonType ? 0 : 10
            wrapMode: Text.Wrap
            text: report.reason
            visible: report.reason
        }

        AccessibleText {
            x: 10
            topPadding: 10
            width: parent.width - 50
            font.pointSize: guiSettings.scaledFont(7/8)
            text: ozoneUtils.actionToDescription(report.lastActionTaken)
            visible: report.lastActionTaken
        }

        AccessibleText {
            x: 10
            topPadding: report.lastActionTaken ? 0 : 10
            width: parent.width - 50
            font.pointSize: guiSettings.scaledFont(7/8)
            text: getStatus()
        }

        AccessibleText {
            x: 10
            width: parent.width - 50
            bottomPadding: 10
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: guiSettings.dateTimeIndication(report.updatedAt)
        }

        Rectangle {
            width: parent.width
            height: 1
            color: guiSettings.separatorColor
        }

        AccessibleText {
            id: endOfListText
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            topPadding: 10
            bottomPadding: 50
            text: qsTr("End of list")
            font.italic: true
            visible: endOfList
        }
    }

    SkySvg {
        height: 40
        width: height
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: (svg.offsetByHeight ? height : 0) - (endOfListText.visible ? endOfListText.height / 2 : 0)
        svg: SvgOutline.navigateNext
        color: guiSettings.textColor
    }

    MouseArea {
        anchors.fill: parent
        onClicked: view.clicked()
    }

    function getStatus() {
        switch (report.status) {
        case QEnums.OZONE_REPORT_STATUS_PENDING:
            return qsTr("Pending")
        case QEnums.OZONE_REPORT_STATUS_RESOLVED:
            return qsTr("Resolved")
        }

        return "Unknown status"
    }
}
