import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    readonly property var userSettings: skywalker ? skywalker.getUserSettings() : null
    required property var videoView // videoview
    property var previewImage
    readonly property bool noSideBar: true

    signal closed

    id: page
    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.fullScreenColor }

    onCover: view.pause()

    Loader {
        id: previewLoader
        active: Boolean(previewImage)

        sourceComponent: Image {
            parent: Overlay.overlay
            x: previewImage.relX
            y: previewImage.relY
            width: previewImage.width
            height: previewImage.height
            fillMode: previewImage.fillMode
            source: previewImage.source
        }
    }

    VideoView {
        property basicprofile nullProfile

        id: view
        y: (parent.height - height) / 2
        width: parent.width
        height: parent.height
        maxHeight: parent.height
        videoView: page.videoView
        contentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
        contentWarning: ""
        contentLabeler: nullProfile
        controlColor: "white"
        disabledColor: "darkslategrey"
        backgroundColor: guiSettings.fullScreenColor
        isFullViewMode: true

        onThumbImageLoaded: previewLoader.active = false
    }

    Rectangle {
        anchors.left: parent.left
        anchors.leftMargin: guiSettings.leftMargin
        anchors.right: parent.right
        anchors.rightMargin: guiSettings.rightMargin
        anchors.top: altFlick.top
        anchors.bottom: parent.bottom
        color: "black"
        opacity: 0.7
        visible: altFlick.visible
    }

    Flickable {
        id: altFlick
        anchors.left: parent.left
        anchors.leftMargin: guiSettings.leftMargin
        anchors.right: parent.right
        anchors.rightMargin: guiSettings.rightMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10 + guiSettings.footerMargin
        width: parent.width
        height: Math.min(contentHeight, 6 * 21)
        clip: true
        contentWidth: parent.width
        contentHeight: altText.contentHeight
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { id: altScrollBar }
        visible: !view.videoPlayingOrPaused && Boolean(videoView.alt)

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
            textFormat: videoView.hasHtmlAlt() ? Text.RichText : Text.PlainText
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
        visible: !view.isPlaying || view.isGif
        onClicked: page.closed()
    }

    SvgButton {
        y: guiSettings.headerMargin
        anchors.right: parent.right
        anchors.rightMargin: guiSettings.rightMargin
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        opacity: 0.7
        svg: SvgOutline.moreVert
        accessibleName: qsTr("more options")
        visible: !view.isPlaying || view.isGif
        onClicked: moreMenu.open()

        SkyMenu {
            id: moreMenu

            AccessibleMenuItem {
                text: qsTr("Save video")
                svg: SvgOutline.save
                onTriggered: root.saveVideo(view.videoSource, videoView.playlistUrl)
            }

            AccessibleMenuItem {
                text: qsTr("Translate")
                svg: SvgOutline.googleTranslate
                enabled: Boolean(videoView.alt)
                onTriggered: root.translateText(videoView.alt)
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
        displayUtils.setNavigationBarColorAndMode("transparent", false)
        displayUtils.setStatusBarTransparentAndMode(true, guiSettings.fullScreenColor, false)
    }

    function resetSystemBarsColor() {
        // As GuiSettings are temporarily set to dark on this page, we determine
        // the normal background color here instead of taking it from guiSettings
        const backgroundColor = userSettings ? userSettings.backgroundColor : Material.background
        displayUtils.setNavigationBarColor(backgroundColor)
        displayUtils.setStatusBarTransparent(false, backgroundColor)
    }

    function cancel() {
        closed()
    }

    Component.onDestruction: {
        resetSystemBarsColor()
    }

    Component.onCompleted: {
        guiSettings.updateScreenMargins()
        setSystemBarsColor()
    }
}
