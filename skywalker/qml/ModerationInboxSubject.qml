import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    required property ozonesubjectviewdetail subject
    readonly property ozoneactionsummary latestActionSummary: ozoneUtils.getLatestActionSummary(subject)
    readonly property string sideBarTitle: qsTr("Notice")
    property basicprofile moderationProfile
    property string sideBarSubTitle: moderationProfile.name
    readonly property SvgImage sideBarSvg: SvgOutline.report
    readonly property int margin: 10

    signal closed
    signal appealed(ozonesubjectview newSubject)

    id: page

    header: SimpleHeader {
        text: sideBarTitle
        subTitle: sideBarSubTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    Flickable {
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: contentItem.childrenRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

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
                visible: !latestActionSummary.isNull()
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

            AccessibleText {
                x: 10
                width: parent.width - 20
                topPadding: 10
                text: ozoneUtils.getAppealStatus(subject.appeal)
                visible: text
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

                onClicked: appeal()
            }

            AccessibleText {
                width: parent.width
                topPadding: margin
                leftPadding: margin
                rightPadding: margin
                font.bold: true
                font.pointSize: guiSettings.scaledFont(9/8)
                text: qsTr("Prior actions")
                visible: subject.actions.length > 1
            }

            Repeater {
                model: subject.actions.slice(1)

                Column {
                    required property ozoneactionview modelData
                    required property int index
                    readonly property ozoneactionsummary actionSummary: ozoneUtils.getActionSummary(subject, index + 1)

                    width: page.width

                    AccessibleText {
                        width: parent.width
                        topPadding: margin
                        leftPadding: margin
                        rightPadding: margin
                        font.bold: true
                        text: actionSummary.subject
                        visible: !actionSummary.isNull()
                    }
                    AccessibleText {
                        x: 10
                        width: parent.width - 20
                        topPadding: 10
                        color: Material.color(Material.Grey)
                        font.pointSize: guiSettings.scaledFont(7/8)
                        text: guiSettings.dateTimeIndication(modelData.createdAt)
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
                        color: Material.color(Material.Grey)
                        font.pointSize: guiSettings.scaledFont(7/8)
                        text: qsTr(`Reversed: ${guiSettings.dateTimeIndication(modelData.reversedAt)}`)
                        visible: !isNaN(modelData.reversedAt.getTime())
                    }

                    Item {
                        width: parent.width
                        height: 10
                    }

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: guiSettings.separatorColor
                    }
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: ozoneUtils.appealInProgress
    }

    OzoneUtils {
        id: ozoneUtils
        skywalker: page.skywalker

        onAppealOk: (newSubject) => {
                skywalker.showStatusMessage(qsTr("Appealed"), QEnums.STATUS_LEVEL_INFO)
                appealed(newSubject)
        }
        onAppealFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker

        onBasicProfileOk: (profile) => moderationProfile = profile
    }

    function appeal() {
        let component = guiSettings.createComponent("ModerationAppeal.qml")
        let view = component.createObject(page, {
                decision: latestActionSummary.subject,
                moderationProfile: moderationProfile
        })
        view.onSendAppeal.connect((reason) => {
                ozoneUtils.appeal(subject, reason)
                root.popStack()
        })
        view.onClosed.connect(() => { root.popStack() })
        root.pushStack(view)
    }

    Component.onCompleted: {
        profileUtils.getBasicProfile(subject.source)
    }
}
