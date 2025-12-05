import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property string imageUrl
    property string imageTitle
    readonly property bool noSideBar: true
    property var previewImage

    signal closed

    id: page
    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.fullScreenColor }

    Loader {
        id: animatedImageLoader
        active: Boolean(previewImage)

        sourceComponent: AnimatedImage {
            x: previewImage.relX
            y: previewImage.relY
            width: previewImage.width
            height: previewImage.height
            fillMode: previewImage.fillMode
            source: previewImage.source

            onStatusChanged: {
                if (status == Image.Ready)
                    previewLoader.active = false
            }
        }
    }

    // It takes a bit foor the animated image is fully loaded. While loading
    // a still image will be shown.
    Loader {
        id: previewLoader
        active: Boolean(previewImage)

        sourceComponent: Image {
            x: previewImage.relX
            y: previewImage.relY
            width: previewImage.width
            height: previewImage.height
            fillMode: previewImage.fillMode
            source: previewImage.source
        }
    }

    AccessibleText {
        id: altText
        anchors.left: parent.left
        anchors.leftMargin: guiSettings.leftMargin
        anchors.right: parent.right
        anchors.rightMargin: guiSettings.rightMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: guiSettings.footerMargin
        padding: 10
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        color: "white"
        text: imageTitle
        visible: imageTitle
    }

    Loader {
        id: imgLoader
        active: !Boolean(previewImage)

        sourceComponent: AnimatedImageAutoRetry {
            id: img
            y: (page.height - altText.height - height) / 2
            width: page.width
            height: page.height - altText.height
            fillMode: Image.PreserveAspectFit
            source: imageUrl
        }
    }

    BusyIndicator {
        id: gifLoadingIndicator
        anchors.centerIn: parent
        running: (imgLoader.item && imgLoader.item.status === Image.Loading) ||
                 (animatedImageLoader.item && animatedImageLoader.item.status === Image.Loading)
    }

    SvgButton {
        x: guiSettings.leftMargin + 10
        y: guiSettings.headerMargin
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        opacity: 0.7
        svg: SvgOutline.arrowBack
        accessibleName: qsTr("go back")
        onClicked: page.closed()
    }

    function setSystemBarsColor() {
        displayUtils.setNavigationBarColorAndMode(guiSettings.fullScreenColor, false)
        displayUtils.setStatusBarColorAndMode(guiSettings.fullScreenColor, false)
    }

    function resetSystemBarsColor() {
        displayUtils.setNavigationBarColor(guiSettings.backgroundColor)
        displayUtils.setStatusBarColor(guiSettings.headerColor)
    }

    function cancel() {
        closed()
    }

    Component.onDestruction: {
        resetSystemBarsColor()
    }

    Component.onCompleted: {
        setSystemBarsColor()
    }
}
