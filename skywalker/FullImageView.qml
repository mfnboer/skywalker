import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

SkyPage {
    required property var images // list<imageview>: var to allow regular javascript arrays
    required property int imageIndex
    property var previewImage
    property bool showControls: true

    signal closed
    signal saveImage(string sourceUrl)
    signal shareImage(string sourceUrl)

    id: page
    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.fullScreenColor }

    SwipeView {
        property bool zooming: currentItem ? currentItem.zooming : false // qmllint disable missing-property

        id: view
        anchors.fill: parent
        currentIndex: imageIndex
        interactive: !zooming

        Repeater {
            model: images.length

            Rectangle {
                required property int index
                property bool isCurrentItem: SwipeView.isCurrentItem
                property bool zooming: img.zooming

                id: imgRect
                color: guiSettings.fullScreenColor

                MouseArea {
                    width: parent.width
                    height: parent.height
                    onClicked: showControls = !showControls
                    onDoubleClicked: (mouse) => {
                        showControls = !showControls
                        img.toggleFullScale(mouse.x, mouse.y)
                    }
                }
                Flickable {
                    id: altFlick
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: altText.bottomMargin
                    width: parent.width
                    height: Math.min(contentHeight, altText.maxHeight)
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

                    ImageAltText {
                        id: altText
                        alt: images[index].alt
                        visible: alt && isCurrentItem
                    }
                }
                ImageWithZoom {
                    property int altHeight: altText.visible ? altFlick.height + 10 : 0

                    id: img
                    y: (parent.height - altHeight - height) / 2
                    width: parent.width
                    height: parent.height - altHeight
                    fillMode: Image.PreserveAspectFit
                    source: images[index].fullSizeUrl
                    reloadIconColor: "white"

                    onStatusChanged: {
                        if (status == Image.Ready)
                            previewLoader.active = false
                    }
                }
            }
        }
    }

    SvgButton {
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        opacity: 0.7
        svg: SvgOutline.arrowBack
        accessibleName: qsTr("go back")
        visible: showControls
        onClicked: page.closed()
    }

    SvgButton {
        anchors.top: parent.top
        anchors.right: parent.right
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        opacity: 0.7
        svg: SvgOutline.moreVert
        accessibleName: qsTr("more options")
        visible: showControls
        onClicked: moreMenu.open()

        Menu {
            id: moreMenu
            modal: true

            MenuItem {
                text: qsTr("Save picture")
                onTriggered: page.saveImage(images[view.currentIndex].fullSizeUrl)

                MenuItemSvg { svg: SvgOutline.save }
            }

            MenuItem {
                text: qsTr("Share picture")
                onTriggered: page.shareImage(images[view.currentIndex].fullSizeUrl)

                MenuItemSvg { svg: SvgOutline.share }
            }

            MenuItem {
                text: qsTr("Translate")
                enabled: images[view.currentIndex].alt
                onTriggered: root.translateText(images[view.currentIndex].alt)

                MenuItemSvg { svg: SvgOutline.googleTranslate }
            }
        }
    }

    Loader {
        id: previewLoader
        active: Boolean(previewImage)

        sourceComponent: Image {
            parent: Overlay.overlay
            x: previewImage.x
            y: previewImage.y
            width: previewImage.width
            height: previewImage.height
            fillMode: previewImage.fillMode
            source: previewImage.source
        }
    }

    DisplayUtils {
        id: displayUtils
        skywalker: root.getSkywalker()
    }

    function getAltHeight() {
        return img.altHeight
    }

    function setSystemBarsColor() {
        displayUtils.setNavigationBarColorAndMode(guiSettings.fullScreenColor, false)
        displayUtils.setStatusBarColorAndMode(guiSettings.fullScreenColor, false)
    }

    function resetSystemBarsColor() {
        displayUtils.setNavigationBarColor(guiSettings.backgroundColor)
        displayUtils.setStatusBarColor(guiSettings.headerColor)
    }

    Component.onDestruction: {
        resetSystemBarsColor()
    }

    Component.onCompleted: {
        setSystemBarsColor()
    }
}
