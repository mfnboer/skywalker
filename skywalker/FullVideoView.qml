import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    readonly property var userSettings: skywalker ? skywalker.getUserSettings() : null
    required property var videoView // videoview
    readonly property bool noSideBar: true

    signal closed

    id: page
    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.fullScreenColor }
    leftPadding: guiSettings.leftMargin
    rightPadding: guiSettings.rightMargin

    onCover: view.pause()

    footer: DeadFooterMargin {
        color: guiSettings.fullScreenColor
    }

    VideoView {
        id: view
        y: (parent.height - height) / 2
        width: parent.width
        height: parent.height
        maxHeight: parent.height
        videoView: page.videoView
        contentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
        contentWarning: ""
        controlColor: "white"
        disabledColor: "darkslategrey"
        backgroundColor: guiSettings.fullScreenColor
        isFullViewMode: true
    }

    Rectangle {
        width: parent.width
        anchors.top: altFlick.top
        anchors.bottom: parent.bottom
        color: "black"
        opacity: 0.7
        visible: altFlick.visible
    }

    Flickable {
        id: altFlick
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        width: parent.width
        height: Math.min(contentHeight, 6 * 21)
        clip: true
        contentWidth: parent.width
        contentHeight: altText.contentHeight
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { id: altScrollBar }
        visible: !view.videoPlayingOrPaused

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
        x: guiSettings.leftMargin
        y: guiSettings.headerMargin
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        opacity: 0.7
        svg: SvgOutline.arrowBack
        accessibleName: qsTr("go back")
        visible: !view.isPlaying
        onClicked: page.closed()
    }

    SvgButton {
        y: guiSettings.headerMargin
        anchors.right: parent.right
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        opacity: 0.7
        svg: SvgOutline.moreVert
        accessibleName: qsTr("more options")
        visible: !view.isPlaying
        onClicked: moreMenu.open()

        Menu {
            id: moreMenu
            modal: true

            MenuItem {
                text: qsTr("Save video")
                onTriggered: root.saveVideo(view.videoSource, videoView.playlistUrl)

                MenuItemSvg {
                    svg: SvgOutline.save
                }
            }

            MenuItem {
                text: qsTr("Translate")
                enabled: Boolean(videoView.alt)
                onTriggered: root.translateText(videoView.alt)

                MenuItemSvg {
                    svg: SvgOutline.googleTranslate
                }
            }
        }
    }

    GuiSettings {
        id: guiSettings
        isLightMode: false
        backgroundColor: guiSettings.fullScreenColor
        textColor: "white"
    }

    function setSystemBarsColor() {
        displayUtils.setNavigationBarColorAndMode(guiSettings.fullScreenColor, false)
        displayUtils.setStatusBarColorAndMode(guiSettings.fullScreenColor, false)
    }

    function resetSystemBarsColor() {
        // As GuiSettings are temporarily set to dark on this page, we determine
        // the normal background color here instead of taking it from guiSettings
        const backgroundColor = userSettings ? userSettings.backgroundColor : Material.background
        displayUtils.setNavigationBarColor(backgroundColor)
        displayUtils.setStatusBarColor(backgroundColor)
    }

    Component.onDestruction: {
        resetSystemBarsColor()
    }

    Component.onCompleted: {
        setSystemBarsColor()
    }
}
