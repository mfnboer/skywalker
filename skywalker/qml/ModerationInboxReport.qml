import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    required property ozonereportview report
    required property ozoneresolutionview resolution
    required property basicprofile author
    required property string authorError
    readonly property string sideBarTitle: qsTr("Your report")
    property string sideBarSubTitle
    readonly property SvgImage sideBarSvg: SvgOutline.report
    readonly property int margin: 10

    signal closed

    id: page

    header: SimpleHeader {
        text: sideBarTitle
        subTitle: sideBarSubTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    Column {
        width: parent.width
        spacing: 0

        AccessibleText {
            width: parent.width
            topPadding: margin
            leftPadding: margin
            rightPadding: margin
            font.bold: true
            font.pointSize: guiSettings.scaledFont(9/8)
            text: getResolutionText()
        }

        AccessibleText {
            width: parent.width
            leftPadding: margin
            rightPadding: margin
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: guiSettings.dateTimeIndication(!resolution.isNull() ? resolution.resolvedAt : report.updatedAt)
        }

        AccessibleText {
            width: parent.width
            padding: margin
            font.bold: true
            text: qsTr("You reported")
        }

        ModerationRecord {
            x: margin
            width: parent.width - 20
            ozoneEntity: report
            author: page.author
            authorError: page.authorError
            showSubjectType: true
        }

        AccessibleText {
            width: parent.width
            topPadding: margin
            leftPadding: margin
            rightPadding: margin
            font.bold: true
            text: qsTr("Reason:")
        }

        AccessibleText {
            x: margin
            width: parent.width - 2 * margin
            text: report.reasonType
            visible: report.reasonType
        }

        AccessibleText {
            x: margin
            width: parent.width - 2 * margin
            topPadding: report.reasonType ? 0 : margin
            wrapMode: Text.Wrap
            text: report.reason
            visible: report.reason
        }

    }

    OzoneUtils {
        id: ozoneUtils
        skywalker: page.skywalker
    }

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker

        onBasicProfileOk: (profile) => sideBarSubTitle = profile.name
    }

    function getResolutionText() {
        if (resolution.isNull())
            return qsTr("Pending")

        if (resolution.outcome === QEnums.OZONE_RESOLUTION_OUTCOME_NO_ACTION)
            return qsTr("No action taken")

        if (resolution.outcome === QEnums.OZONE_RESOLUTION_OUTCOME_OTHER || resolution.actionTaken === QEnums.OZONE_RESOLUTION_OUTCOME_UNKNOWN)
            return qsTr("Resolved")

        return ozoneUtils.actionToDescription(resolution.actionTaken)
    }

    Component.onCompleted: {
        profileUtils.getBasicProfile(report.source)
    }
}
