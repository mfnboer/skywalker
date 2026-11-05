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
                    text: ozoneUtils.subjectTypeToString(report)
                }

                PostHeaderWithAvatar {
                    x: margin
                    bottomPadding: margin
                    width: parent.width - 2 * margin
                    userDid: skywalker.getUserDid()
                    author: page.author
                }

                AccessibleText {
                    x: margin
                    width: parent.width - 2 * margin
                    wrapMode: Text.Wrap
                    text: report.recordContent.text
                    visible: !report.recordContent.isNull()
                }

                AccessibleText {
                    width: parent.width
                    leftPadding: margin
                    rightPadding: margin
                    bottomPadding: margin
                    color: Material.color(Material.Grey)
                    font.pointSize: guiSettings.scaledFont(7/8)
                    text: guiSettings.dateTimeIndication(report.recordContent.createdAt)
                    visible: !report.recordContent.isNull()
                }

                AccessibleText {
                    x: margin
                    bottomPadding: margin
                    width: parent.width - 2 * margin
                    color: guiSettings.linkColor
                    text: qsTr(`View ${ozoneUtils.subjectTypeToString(report, false)}`)
                    visible: mustShowViewLink()


                    MouseArea {
                        anchors.fill: parent
                        onClicked: openSubject()
                    }
                }
            }
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

    function mustShowViewLink() {
        if (report.subjectType === QEnums.OZONE_REPORT_SUBJECT_UNKNOWN)
            return false

        if (report.subjectType !== QEnums.OZONE_REPORT_SUBJECT_STRONG_REF)
            return true

        return ![QEnums.REF_TYPE_ACTOR_STATUS,
                 QEnums.REF_TYPE_ACTOR_LINK,
                 QEnums.REF_TYPE_UNKNOWN].includes(ozoneUtils.subjectToRefType(report))
    }

    function openSubject() {
        console.debug("Open subject type:", report.subjectType, "subject:", report.subject, "subject uri:", report.subjectUri)

        if (report.subjectType === QEnums.OZONE_REPORT_SUBJECT_STRONG_REF) {
            const refType = ozoneUtils.subjectToRefType(report)

            if (refType === QEnums.REF_TYPE_LIST) {
                root.viewListByUri(report.subjectUri, false)
                return
            }

            if (refType === QEnums.REF_TYPE_FEED) {
                skywalker.getFeedGenerator(report.subjectUri, false)
                return
            }
        }
        else if (report.subjectType === QEnums.OZONE_REPORT_SUBJECT_CONVO_REF) {
            const convoRef = report.subject
            skywalker.chat.startConvoForConvoId(convoRef.convoId)
            return
        }
        else if (report.subjectType === QEnums.OZONE_REPORT_SUBJECT_MESSAGE_REF) {
            const msgRef = report.subject
            skywalker.chat.startConvoForConvoId(msgRef.convoId, msgRef.messageId)
        }

        if (!report.subjectUri)
            return

        if (report.subjectUri.startsWith("did:")) {
            root.openLink(report.subjectUri)
            return
        }

        const httpsUrl = root.getLinkUtils().toHttpsLink(report.subjectUri)

        if (httpsUrl)
            root.openLink(httpsUrl)
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
