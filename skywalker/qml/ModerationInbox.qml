import QtQuick
import QtQuick.Controls
import skywalker
import atproto.lib

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    required property int reportModelId
    required property int subjectModelId
    property ozoneaccountstatus accountStatus
    property basicprofile moderationProfile
    readonly property string sideBarTitle: qsTr("Moderation inbox")
    readonly property SvgImage sideBarSvg: SvgOutline.inbox

    signal closed

    id: page

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    SkyTabBar {
        id: tabBar

        AccessibleTabButton {
            text: qsTr("Your reports")
        }
        AccessibleTabButton {
            text: qsTr(`${accountStatus.standingSymbol} Your account`).trim()
        }
    }

    Rectangle {
        id: tabBarSeperator
        anchors.top: tabBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    SwipeView {
        id: inboxSwipeView
        anchors.top: tabBarSeperator.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: tabBar.currentIndex

        onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)

        SkyListView {
            id: reportsView
            clip: true
            model: skywalker.getOzoneReportListModel(reportModelId)
            preloadNextPageFunc: () => ozoneUtils.getReportListNextPage(reportModelId)

            SwipeView.onIsCurrentItemChanged: {
                if (!SwipeView.isCurrentItem)
                    updateSeen()
            }

            delegate: ModerationInboxReportDelegate {
                width: page.width
                ozoneUtils: getOzoneUtils()

                onClicked: ozoneUtils.getReport(report.id, author)
            }

            FlickableRefresher {
                inProgress: Boolean(reportsView.model?.getFeedInProgress)
                topOvershootFun: () => {
                                     reportsView.updateSeen()
                                     ozoneUtils.getReportList(reportModelId)
                                 }
                bottomOvershootFun: () => ozoneUtils.getReportListNextPage(reportModelId)
                topText: qsTr("Refresh reports")
            }

            EmptyListIndication {
                y: 50
                svg: SvgOutline.reportOff
                text: qsTr("No reports")
                list: reportsView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: Boolean(reportsView.model?.getFeedInProgress)
            }

            function updateSeen() {
                if (model.hasUnread())
                    ozoneUtils.updateSeen(OzoneInboxSection.REPORTS, model.getLatestUpdatedAt())
            }
        }

        SkyListView {
            id: subjectsView
            clip: true
            model: skywalker.getOzoneSubjectListModel(subjectModelId)
            preloadNextPageFunc: () => ozoneUtils.getSubjectListNextPage(subjectModelId)

            SwipeView.onIsCurrentItemChanged: {
                if (!SwipeView.isCurrentItem)
                    updateSeen()
            }

            header: Rectangle {
                z: guiSettings.headerZLevel
                width: page.width
                height: visible ? headerContent.height : 0
                color: guiSettings.backgroundColor
                visible: accountStatus.hasActiveStrikes || accountStatus.standing !== QEnums.OZONE_ACCOUNT_STANDING_GOOD

                Column {
                    id: headerContent
                    width: parent.width
                    spacing: 0

                    Item {
                        width: parent.width
                        height: 10
                    }
                    AccessibleText {
                        x: 10
                        width: parent.width - 20
                        bottomPadding: 5
                        text: moderationProfile.name
                    }
                    AccessibleText {
                        x: 10
                        width: parent.width - 20
                        bottomPadding: 5
                        font.bold: true
                        text: qsTr("Your account has strikes against it")
                        visible: accountStatus.hasActiveStrikes
                    }
                    AccessibleText {
                        x: 10
                        width: parent.width - 20
                        font.pointSize: guiSettings.scaledFont(7/8)
                        text: qsTr(`${accountStatus.standingSymbol} ${QEnums.ozoneAccountStandingToString(accountStatus.standing, accountStatus.rawStanding)}`)
                        visible: accountStatus.standing !== QEnums.OZONE_ACCOUNT_STANDING_GOOD
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
            headerPositioning: ListView.OverlayHeader

            delegate: ModerationInboxSubjectDelegate {
                width: page.width
                ozoneUtils: getOzoneUtils()

                onClicked: ozoneUtils.getSubject(subject.subjectUri)
            }

            FlickableRefresher {
                inProgress: Boolean(subjectsView.model?.getFeedInProgress)
                topOvershootFun: () => {
                                     subjectsView.updateSeen()
                                     ozoneUtils.getSubjectList(subjectModelId)
                                 }
                bottomOvershootFun: () => ozoneUtils.getSubjectListNextPage(subjectModelId)
                topText: qsTr("Refresh")
            }

            EmptyListIndication {
                y: 50 + (parent.headerItem ? parent.headerItem.height : 0)
                svg: SvgOutline.reportOff
                text: qsTr("No actions")
                list: subjectsView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: Boolean(subjectsView.model?.getFeedInProgress)
            }

            function updateSeen() {
                if (model.hasUnread())
                    ozoneUtils.updateSeen(OzoneInboxSection.SUBJECTS, model.getLatestUpdatedAt())
            }
        }
    }

    OzoneUtils {
        id: ozoneUtils
        skywalker: page.skywalker

        onReportOk: (report, resolution, profile) => viewReport(report, resolution, profile)
        onReportFailed: (reportId, error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onSubjectOk: (subject) => viewSubject(subject)
        onSubjectFailed: (subject, error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onAccountStatus: (status) => {
                page.accountStatus = status
                profileUtils.getBasicProfile(page.accountStatus.source)
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker

        onBasicProfileOk: (profile) => moderationProfile = profile
    }

    function getOzoneUtils() {
        return ozoneUtils
    }

    function viewReport(report, resolution, profile) {
        let component = guiSettings.createComponent("ModerationInboxReport.qml")
        let view = component.createObject(page, {
                report: report,
                resolution: resolution,
                author: profile
        })
        view.onClosed.connect(() => { root.popStack() })
        root.pushStack(view)
    }

    function viewSubject(subject) {
        let component = guiSettings.createComponent("ModerationInboxSubject.qml")
        let view = component.createObject(page, {
                subject: subject
        })
        view.onAppealed.connect((newSubject) => {
                subjectsView.model.updateSubject(newSubject)
                root.popStack()
        })
        view.onClosed.connect(() => { root.popStack() })
        root.pushStack(view)
    }

    function showAccountTab() {
        tabBar.setCurrentIndex(1)
    }

    Component.onDestruction: {
        inboxSwipeView.currentItem.updateSeen()
        skywalker.removeOzoneReportListModel(reportModelId)
        skywalker.removeOzoneSubjectListModel(subjectModelId)
    }

    Component.onCompleted: {
        ozoneUtils.getReportList(reportModelId)
        ozoneUtils.getSubjectList(subjectModelId)
        ozoneUtils.getAccountStatus()
    }
}
