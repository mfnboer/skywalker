import QtQuick
import QtQuick.Layouts
import skywalker

SkyListView {
    required property list<profilelinkview> links
    property Skywalker skywalker: root.getSkywalker()

    // If we want to use this list for more purposes, we should parameterize the title
    readonly property string sideBarTitle: qsTr("Report a link")
    readonly property string sideBarSubTitle: qsTr("Which link do you want to report?")
    readonly property SvgImage sideBarSvg: SvgOutline.globe

    signal closed
    signal clicked(profilelinkview link)

    id: page
    model: links
    boundsBehavior: Flickable.StopAtBounds
    spacing: 5

    header: SimpleHeader {
        text: sideBarTitle
        subTitle: sideBarSubTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: Item {
        required property profilelinkview modelData
        required property int index

        width: page.width
        height: col.height

        Column {
            id: col
            width: parent.width

            ProfileLinkRow {
                x: 10
                width: parent.width - 20
                link: modelData
            }

            Item {
                width: parent.width
                height: 5
            }

            Rectangle {

                width: parent.width
                height: 1
                color: guiSettings.separatorColor
                visible: index < links.length - 1
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: page.clicked(modelData)
        }
    }
}
