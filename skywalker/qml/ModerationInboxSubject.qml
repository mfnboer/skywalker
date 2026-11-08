import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    required property ozonesubjectviewdetail subject
    readonly property ozoneactionsummary latestActionSummary: ozoneUtils.getLatestActionSummary(subject)
    readonly property string sideBarTitle: qsTr("Notice")
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
            text: latestActionSummary.subject
            visible: !subject.latestAction.isNull()
        }
        AccessibleText {
            x: 10
            width: parent.width - 20
            topPadding: 10
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: guiSettings.dateTimeIndication(subject.updatedAt)
        }

        ModerationRecord {
            x: margin
            width: parent.width - 20
            ozoneEntity: subject
            author: skywalker.user
        }

        ContentLabels {
            x: 10
            userDid: skywalker.getUserDid()
            contentAuthor: skywalker.getUser()
            contentLabels: latestActionSummary.labels
        }

        ModerationPolicySummaryList {
            x: 10
            width: parent.width - 20
            summary: latestActionSummary.policies
        }

        Item {
            width: parent.width
            height: appealButton.visible ? 10 : 0
        }

        SkyButton {
            id: appealButton
            x: 10
            height: 40
            text: qsTr("Appeal this decision")
            visible: subject.canAppeal()
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

    Component.onCompleted: {
        profileUtils.getBasicProfile(subject.source)
    }
}
