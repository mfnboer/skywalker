import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property ozonereportview report
    required property basicprofile author
    required property bool endOfList

    id: view

    Column {
        x: 10
        width: parent.width
        spacing: 0

        AccessibleText {
            x: 10
            width: parent.width - 20
            font.bold: true
            elide: Text.ElideRight
            text: getSubjectType()
        }

        AccessibleText {
            x: 10
            width: parent.width - 20
            text: report.reasonType
            visible: report.reasonType
        }

        AccessibleText {
            x: 10
            width: parent.width - 20
            topPadding: 10
            wrapMode: Text.Wrap
            text: report.reason
            visible: report.reason
        }

        AuthorNameAndStatus {
            x: 10
            topPadding: 10
            width: parent.width - 20
            userDid: skywalker.getUserDid()
            author: view.author
        }

        AccessibleText {
            x: 10
            width: parent.width - 20
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            text: report.lastActionTaken
            visible: report.lastActionTaken
        }

        AccessibleText {
            x: 10
            width: parent.width - 20
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            text: getStatus()
        }

        AccessibleText {
            x: 10
            width: parent.width - 20
            bottomPadding: 10
            elide: Text.ElideRight
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
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            topPadding: 10
            bottomPadding: 50
            elide: Text.ElideRight
            text: qsTr("End of list")
            font.italic: true
            visible: endOfList
        }
    }

    function getSubjectType() {
        switch (report.subjectType) {
        case QEnums.OZONE_REPORT_SUBJECT_REPO_REF:
            return qsTr("Account")
        case QEnums.OZONE_REPORT_SUBJECT_STRONG_REF:
            return qsTr("TODO")
        case QEnums.OZONE_REPORT_SUBJECT_CONVO_REF:
            return qsTr("Chat")
        case QEnums.OZONE_REPORT_SUBJECT_MESSAGE_REF:
            return qsTr("Message")
        }

        return "Unknown subject"
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
