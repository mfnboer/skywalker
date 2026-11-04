import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    required property ozonereportview report
    required property ozoneresolutionview resolution
    required property basicprofile author
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
            elide: Text.ElideRight
            text: getResolutionText()
        }

        AccessibleText {
            width: parent.width
            leftPadding: margin
            rightPadding: margin
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: guiSettings.dateTimeIndication(report.updatedAt)
        }

        AccessibleText {
            width: parent.width
            padding: margin
            font.bold: true
            elide: Text.ElideRight
            text: qsTr("You reported")
        }

        Rectangle {
            x: margin
            radius: guiSettings.radius
            width: parent.width - 20
            height: contentCol.height
            color: guiSettings.postHighLightColor
            border.width: 1
            border.color: guiSettings.borderHighLightColor

            Column {
                id: contentCol
                width: parent.width
                spacing: 0

                AccessibleText {
                    padding: margin
                    width: parent.width
                    font.bold: true
                    elide: Text.ElideRight
                    text: ozoneUtils.subjectTypeToString(report)
                }

                PostHeaderWithAvatar {
                    x: margin
                    topPadding: margin
                    bottomPadding: margin
                    width: parent.width - 2 * margin
                    userDid: skywalker.getUserDid()
                    author: page.author
                }

                // TODO record content
            }
        }

        AccessibleText {
            width: parent.width
            topPadding: margin
            leftPadding: margin
            rightPadding: margin
            elide: Text.ElideRight
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
            topPadding: margin
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
