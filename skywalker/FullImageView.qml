import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

SkyPage {
    required property var images // list<imageview>: var to allow regular javascript arrays
    required property int imageIndex
    property bool showControls: true

    signal closed
    signal saveImage(string sourceUrl)
    signal shareImage(string sourceUrl)

    id: page
    width: parent.width
    height: parent.height
    padding: 10
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
                        plainText: images[index].alt
                        visible: images[index].alt && isCurrentItem
                    }
                }
                ImageWithZoom {
                    id: img
                    y: (parent.height - altFlick.height - height) / 2
                    width: parent.width
                    height: parent.height - altFlick.height
                    fillMode: Image.PreserveAspectFit
                    source: images[index].fullSizeUrl
                    reloadIconColor: "white"
                }

                MouseArea {
                    width: parent.width
                    height: parent.height
                    onClicked: showControls = !showControls
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

    DisplayUtils {
        id: displayUtils
        skywalker: root.getSkywalker()
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
