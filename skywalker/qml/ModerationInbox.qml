import QtQuick
import QtQuick.Controls
import skywalker
import atproto.lib

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    required property int reportModelId
    required property int subjectModelId
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
            text: qsTr("Your account")
        }
    }

    SwipeView {
        id: inboxSwipeView
        anchors.top: tabBar.bottom
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
                                     updateSeen()
                                     ozoneUtils.getReportList(reportModelId)
                                 }
                bottomOvershootFun: () => ozoneUtils.getReportListNextPage(reportModelId)
                topText: qsTr("Refresh reports")
            }

            EmptyListIndication {
                y: parent.headerItem ? parent.headerItem.height : 0
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

            delegate: ModerationInboxSubjectDelegate {
                width: page.width
                ozoneUtils: getOzoneUtils()

                onClicked: ozoneUtils.getSubject(subject.subjectUri)
            }

            FlickableRefresher {
                inProgress: Boolean(subjectsView.model?.getFeedInProgress)
                topOvershootFun: () => {
                                     updateSeen()
                                     ozoneUtils.getSubjectList(subjectModelId)
                                 }
                bottomOvershootFun: () => ozoneUtils.getSubjectListNextPage(subjectModelId)
                topText: qsTr("Refresh")
            }

            EmptyListIndication {
                y: parent.headerItem ? parent.headerItem.height : 0
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
        view.onClosed.connect(() => { root.popStack() })
        root.pushStack(view)
    }

    Component.onDestruction: {
        inboxSwipeView.currentItem.updateSeen()
        skywalker.removeOzoneReportListModel(reportModelId)
        skywalker.removeOzoneSubjectListModel(subjectModelId)
    }

    Component.onCompleted: {
        ozoneUtils.getReportList(reportModelId)
        ozoneUtils.getSubjectList(subjectModelId)
    }
}
