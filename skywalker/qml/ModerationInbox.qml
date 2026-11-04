import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    required property int reportsModelId
    readonly property string sideBarTitle: qsTr("Reports")
    readonly property SvgImage sideBarSvg: SvgOutline.report

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
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: tabBar.currentIndex

        onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)

        SkyListView {
            id: reportsView
            clip: true
            model: skywalker.getOzoneReportListModel(reportsModelId)
            preloadNextPageFunc: () => ozoneUtils.getReportListNextPage(reportsModelId)

            delegate: ModerationInboxDelegate {
                width: page.width
                ozoneUtils: getOzoneUtils()

                onClicked: ozoneUtils.getReport(report.id, author)
            }

            FlickableRefresher {
                inProgress: Boolean(reportsView.model?.getFeedInProgress)
                topOvershootFun: () => ozoneUtils.getReportList(reportsModelId)
                bottomOvershootFun: () => ozoneUtils.getReportListNextPage(reportsModelId)
                topText: qsTr("Refresh lists")
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
        }
    }

    OzoneUtils {
        id: ozoneUtils
        skywalker: page.skywalker

        onReportOk: (report, resolution, profile) => viewReport(report, resolution, profile)
        onReportFailed: (reportId, error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
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

    Component.onDestruction: {
        skywalker.removeOzoneReportListModel(reportsModelId)
    }

    Component.onCompleted: {
        ozoneUtils.getReportList(reportsModelId)
    }
}
