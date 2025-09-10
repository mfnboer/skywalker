import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var globalLabelModel
    required property int labelerAuthorListModelId
    property var skywalker: root.getSkywalker()
    property var contentFilter: skywalker.getContentFilter()
    property int margin: 10
    readonly property string sideBarTitle: qsTr("Content Filtering")
    readonly property SvgImage sideBarSvg: SvgOutline.visibility

    signal closed()

    id: page
    padding: 10

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    footer: DeadFooterMargin {}

    Flickable {
        anchors.topMargin: !root.showSideBar ? 0 : guiSettings.headerMargin
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: labelerListView.y + labelerListView.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        HeaderText {
            id: globalContentFilters
            text: qsTr("Global content filters")
        }

        AccessibleCheckBox {
            anchors.top: globalContentFilters.bottom
            anchors.topMargin: 10
            bottomPadding: 20
            id: adultContentSwitch
            width: parent.width
            Material.accent: guiSettings.buttonColor
            text: qsTr("Adult content")
            checked: page.globalLabelModel.adultContent
            onCheckedChanged: page.globalLabelModel.adultContent = checked

            Accessible.role: Accessible.Button
            Accessible.name: text
            Accessible.onPressAction: toggle()
        }

        ListView {
            id: globalLabelListView
            anchors.top: adultContentSwitch.bottom
            width: parent.width
            height: contentHeight
            clip: true
            model: page.globalLabelModel
            boundsBehavior: Flickable.StopAtBounds

            Accessible.role: Accessible.List

            delegate: ContentGroupDelegate {
                width: parent.width
                isSubscribed: true
                adultContent: page.globalLabelModel.adultContent
            }
        }

        HeaderText {
            id: subscribedLabelers
            anchors.top: globalLabelListView.bottom
            text: qsTr("Subscribed to labelers")
        }

        ListView {
            id: labelerListView
            anchors.top: subscribedLabelers.bottom
            anchors.topMargin: 10
            width: parent.width
            height: contentHeight
            clip: true
            model: skywalker.getAuthorListModel(page.labelerAuthorListModelId)
            boundsBehavior: Flickable.StopAtBounds

            Accessible.role: Accessible.List

            delegate: AuthorViewDelegate {
                width: labelerListView.width - 20
                highlight: contentFilter.hasNewLabels(author.did)
                maximumDescriptionLineCount: 3

                SkySvg {
                    height: 40
                    width: height
                    x: parent.width - 20
                    y: (parent.height + height) / 2
                    svg: SvgOutline.navigateNext
                    color: guiSettings.textColor
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        skywalker.saveGlobalContentFilterPreferences();
                        skywalker.getDetailedProfile(author.did)
                        parent.highlight = false
                    }
                }
            }
        }
    }


    function reloadSubscribedLabelers() {
        skywalker.getAuthorList(labelerAuthorListModelId)
    }

    Component.onDestruction: {
        contentFilter.onSubscribedLabelersChanged.disconnect(reloadSubscribedLabelers)

        skywalker.saveGlobalContentFilterPreferences();
        skywalker.removeAuthorListModel(labelerAuthorListModelId)
    }

    Component.onCompleted: {
        contentFilter.onSubscribedLabelersChanged.connect(reloadSubscribedLabelers)
    }
}
