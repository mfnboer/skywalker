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
    }

    Component.onDestruction: {
        skywalker.removeOzoneReportListModel(reportsModelId)
    }

    Component.onCompleted: {
        ozoneUtils.getReportList(reportsModelId)
    }
}
