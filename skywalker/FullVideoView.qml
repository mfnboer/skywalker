import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var videoView // videoview

    signal closed

    id: page
    width: parent.width
    height: parent.height
    padding: 10
    background: Rectangle { color: "black" }

    VideoView {
        y: (parent.height - altFlick.height - height) / 2
        width: parent.width
        maxHeight: parent.height - altFlick.height
        videoView: page.videoView
        contentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
        contentWarning: ""
        controlColor: "white"
        disabledColor: "darkslategrey"
        backgroundColor: "black"
        isFullViewMode: true
    }

    Flickable {
        id: altFlick
        anchors.bottom: parent.bottom
        width: parent.width
        height: Math.min(contentHeight, 6 * 21)
        clip: true
        contentWidth: parent.width
        contentHeight: altText.contentHeight
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { id: altScrollBar }

        onHeightChanged: setScrollBarPolicy()
        onContentHeightChanged: setScrollBarPolicy()

        function setScrollBarPolicy() {
            altScrollBar.policy = contentHeight > height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            altScrollBar.contentItem.color = "#1d3030"
        }

        SkyCleanedText {
            id: altText
            leftPadding: 10
            width: parent.width - 15
            wrapMode: Text.Wrap
            color: "white"
            plainText: videoView.alt
            visible: Boolean(videoView.alt)
        }
    }

    SvgButton {
        iconColor: "white"
        Material.background: "black"
        opacity: 0.7
        svg: svgOutline.arrowBack
        accessibleName: qsTr("go back")
        onClicked: page.closed()
    }

    SvgButton {
        anchors.top: parent.top
        anchors.right: parent.right
        iconColor: "white"
        Material.background: "black"
        opacity: 0.7
        svg: svgOutline.moreVert
        accessibleName: qsTr("more options")
        onClicked: moreMenu.open()
        visible: Boolean(videoView.alt)

        Menu {
            id: moreMenu
            modal: true

            MenuItem {
                text: qsTr("Translate")
                enabled: Boolean(videoView.alt)
                onTriggered: root.translateText(videoView.alt)

                MenuItemSvg {
                    svg: svgOutline.googleTranslate
                }
            }
        }
    }
}
