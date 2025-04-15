import QtQuick
import QtQuick.Controls

Item {
    required property var thumbImage

    signal done(var img)

    id: animator

    Loader {
        id: zoomImage
        active: false

        sourceComponent: Image {
            property point orig: thumbImage.parent.mapToItem(root.contentItem, thumbImage.x, thumbImage.y)

            parent: Overlay.overlay
            x: orig.x
            y: orig.y
            width: thumbImage.width
            height: thumbImage.height
            fillMode: thumbImage.fillMode
            source: thumbImage.source

            onStatusChanged: {
                if (status == Image.Ready)
                    thumbImage.visible = false
            }
        }
    }

    NumberAnimation {
        property point orig
        property int origWidth: root.width
        property int origHeight: root.height
        property int origImplicitWidth: root.width
        property int origImplicitHeight: root.height
        property real zoom: 0.0
        readonly property int marginHeight: altText.alt ? Math.min(altText.contentHeight, altText.maxHeight) + altText.bottomMargin : 0
        readonly property int maxHeight: root.height - marginHeight
        readonly property real scale: Math.min(root.width / origImplicitWidth, maxHeight / origImplicitHeight)
        readonly property real left: (root.width - origImplicitWidth * scale) / 2
        readonly property real right: left + origImplicitWidth * scale
        readonly property real top: (maxHeight - origImplicitHeight * scale) / 2
        readonly property real bottom: top + origImplicitHeight * scale

        id: zoomAnimation
        target: zoomAnimation
        property: "zoom"
        from: 0.0
        to: 1.0
        duration: 200
        easing.type: Easing.InOutQuad

        onStopped: {
            done(zoomImage.item)
            zoomImage.active = false
            thumbImage.visible = true
        }

        onZoomChanged: {
            const newX = orig.x - (orig.x - left) * zoom
            const newY = orig.y - (orig.y - top) * zoom
            zoomImage.item.x = newX
            zoomImage.item.y = newY
            zoomImage.item.width = orig.x + origWidth + (right - orig.x - origWidth) * zoom - newX
            zoomImage.item.height = orig.y + origHeight + (bottom - orig.y - origHeight) * zoom - newY
        }

        function run() {
            orig = thumbImage.parent.mapToItem(root.contentItem, thumbImage.x, thumbImage.y)
            origWidth = thumbImage.width
            origHeight = thumbImage.height
            origImplicitWidth = thumbImage.implicitWidth
            origImplicitHeight = thumbImage.implicitHeight
            zoomImage.active = true
            start()
        }
    }

    ImageAltText {
        id: altText
        parent: Overlay.overlay
        alt: thumbImage.imageView.alt
        visible: false
    }

    Component.onCompleted: {
        zoomAnimation.run()
    }
}
