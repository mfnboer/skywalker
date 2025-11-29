import QtQuick
import QtQuick.Controls

Item {
    required property var thumbImage
    required property string imageAlt
    property bool swipeMode: false
    property bool thumbImageVisible: true
    property var currentPage
    property int headerHeight: 0
    property int footerHeight: 0

    signal done(var img)
    signal reverseDone()

    id: animator

    Loader {
        id: zoomImage
        active: false

        sourceComponent: Rectangle {
            property var img: image
            readonly property real offsetZoom: 1.0 - zoomAnimation.zoom
            readonly property real headerOffset: headerHeight * offsetZoom
            readonly property real footerOffset: footerHeight * offsetZoom

            parent: Overlay.overlay
            y: headerOffset
            width: parent.width
            height: parent.height - headerOffset - footerOffset
            color: "transparent"
            clip: true

            Image {
                property point orig: thumbImage.parent.mapToItem(root.contentItem, thumbImage.x, thumbImage.y)
                property int relX: orig.x
                property int relY: orig.y

                id: image
                x: relX
                y: relY - headerOffset
                width: thumbImage.width
                height: thumbImage.height
                fillMode: thumbImage.fillMode
                source: thumbImage.source

                onStatusChanged: {
                    console.debug("Zoom image status:", status)

                    if (status == Image.Ready) {
                        thumbImage.setVisible(false)
                        zoomAnimation.start()
                    } else if (status == Image.Error) {
                        // Make sure the animation always gets started
                        zoomAnimation.start()
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: image.status == Image.Loading
                }

                function setRelPos(posX, posY) {
                    relX = posX
                    relY = posY
                }
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
        readonly property int marginHeight: Boolean(altText.alt) ? altFlick.height + altText.bottomMargin : 0
        readonly property int maxHeight: root.height - marginHeight
        readonly property real scale: Math.min(root.width / origImplicitWidth, maxHeight / origImplicitHeight)
        readonly property real left: (root.width - origImplicitWidth * scale) / 2
        readonly property real right: left + origImplicitWidth * scale
        readonly property bool alignToTop: swipeMode && (maxHeight - origImplicitHeight * scale) < guiSettings.topAlignmentThreshold
        readonly property real top: alignToTop ? 0 : (maxHeight - origImplicitHeight * scale) / 2
        readonly property real bottom: top + origImplicitHeight * scale

        id: zoomAnimation
        target: zoomAnimation
        property: "zoom"
        from: 0.0
        to: 1.0
        duration: 200
        easing.type: Easing.InOutQuad

        onStopped: {
            root.enablePopupShield(false)

            if (from < to)
                done(zoomImage.item.img)
            else
                reverseDone()

            zoomImage.item.visible = false
            thumbImage.setVisible(thumbImageVisible)
        }

        onZoomChanged: {
            const newX = orig.x - (orig.x - left) * zoom
            const newY = orig.y - (orig.y - top) * zoom
            zoomImage.item.img.setRelPos(newX, newY)
            zoomImage.item.img.width = orig.x + origWidth + (right - orig.x - origWidth) * zoom - newX
            zoomImage.item.img.height = orig.y + origHeight + (bottom - orig.y - origHeight) * zoom - newY
            root.enablePopupShield(true, zoom)
        }

        function run() {
            setCurrentPage()
            orig = thumbImage.parent.mapToItem(root.contentItem, thumbImage.x, thumbImage.y)
            origWidth = thumbImage.width
            origHeight = thumbImage.height
            origImplicitWidth = thumbImage.implicitWidth
            origImplicitHeight = thumbImage.implicitHeight
            zoomImage.active = true
        }

        function reverseRun() {
            setCurrentPage()
            root.enablePopupShield(true, 1.0)
            zoomImage.item.visible = true
            thumbImage.setVisible(false)
            from = 1.0
            to = 0.0
            start()
        }
    }

    // The flickable is needed to calculate the correct height of the alt-text when
    // a scrollbar gets added.
    Flickable {
        id: altFlick
        parent: Overlay.overlay
        anchors.left: parent.left
        anchors.leftMargin: altText.leftMargin
        anchors.right: parent.right
        anchors.rightMargin: altText.rightMargin
        height: Math.min(contentHeight, altText.maxHeight)
        clip: true
        contentWidth: width
        contentHeight: altText.contentHeight
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { id: altScrollBar }
        visible: false

        ImageAltText {
            id: altText
            alt: swipeMode ? "" : imageAlt
        }
    }

    function reverseRun() {
        zoomAnimation.reverseRun()
    }

    function setCurrentPage() {
        currentPage = root.currentStackItem()
        setHeaderHeight()
        setFooterHeight()
    }

    function setHeaderHeight() {
        if (typeof currentPage.getHeaderHeight == 'function')
            headerHeight = currentPage.getHeaderHeight()
        else
            headerHeight = 0

        console.debug("Current page:", currentPage, "header:", headerHeight)
    }

    function setFooterHeight() {
        if (typeof currentPage.getFooterHeight == 'function')
            footerHeight = currentPage.getFooterHeight()
        else
            footerHeight = 0

        console.debug("Current page:", currentPage, "footer:", footerHeight)
    }

    Component.onCompleted: {
        // An image in a RoundedFrame has its visible property set to false
        thumbImageVisible = thumbImage.getVisible()

        zoomAnimation.run()
    }
}
