import QtQuick
import QtQuick.Controls.Material
import skywalker

Item {
    property string topText: ""
    property bool inProgress: false
    property var topOvershootFun: null
    property var bottomOvershootFun: null
    property bool inTopOvershoot: false
    property bool inBottomOvershoot: false
    property var list: parent
    property bool reverseFeed: false
    property int verticalOvershoot: list.verticalOvershoot
    property var scrollToTopFun: reverseFeed ? () => list.positionViewAtEnd() : () => list.positionViewAtBeginning()
    property var scrollToBottomFun: reverseFeed ? () => list.positionViewAtBeginning() : () => list.positionViewAtEnd()
    property bool scrollTopTopButtonVisible: false
    property int scrollToTopButtonMargin: 0
    property bool enableScrollToTop: true
    property bool enableDirectionalScroll: false
    property bool ignoreFooter: false

    anchors.fill: parent
    //z: parent.z - 1

    AccessibleText {
        id: refreshText
        anchors.horizontalCenter: parent.horizontalCenter
        y: calcY()
        font.italic: true
        color: guiSettings.textColor
        visible: reverseFeed ? verticalOvershoot > 0 : verticalOvershoot < 0
        text: ((!reverseFeed && inTopOvershoot) || (reverseFeed && inBottomOvershoot)) ? qsTr("Refreshing...") : topText

        function calcY() {
            if (reverseFeed)
                return parent.height - ((!ignoreFooter && list.footerItem && list.footerItem.visible) ? list.footerItem.height : 0) - verticalOvershoot
            else
                return ((list.headerItem && list.headerItem.visible) ? list.headerItem.height : 0) - verticalOvershoot - height
        }
    }

    SvgButton {
        readonly property bool listIsAtBeginning: (!reverseFeed && list.atYBeginning) || (reverseFeed && list.atYEnd)
        property bool scrollToTop: true
        property SvgImage prevSvg: reverseFeed ? SvgOutline.scrollToBottom : SvgOutline.scrollToTop

        id: scrollToTopButton
        x: 7
        y: parent.height - height - ((!ignoreFooter && list.footerItem && list.footerItem.visible) ? list.footerItem.height : scrollToTopButtonMargin) - 10
        width: 50
        height: width
        iconColor: guiSettings.textColor
        Material.background: guiSettings.buttonNeutralColor
        svg: calcSvg()
        accessibleName: reverseFeed ? qsTr("Scroll to bottom") : qsTr("Scroll to top")
        visible: scrollTopTopButtonVisible && !listIsAtBeginning && enableScrollToTop
        onClicked: {
            if (scrollToTop)
                scrollToTopFun()
            else
                scrollToBottomFun()
        }

        function calcSvg() {
            if (enableDirectionalScroll) {
                if (list.verticalVelocity < 0) {
                    scrollToTop = true
                    prevSvg = reverseFeed ? SvgOutline.scrollToBottom : SvgOutline.scrollToTop
                } else if (list.verticalVelocity > 0) {
                    scrollToTop = false
                    prevSvg = reverseFeed ? SvgOutline.scrollToTop : SvgOutline.scrollToBottom
                }

                return prevSvg
            } else {
                return reverseFeed ? SvgOutline.scrollToBottom : SvgOutline.scrollToTop
            }
        }

        function calcY() {
            if (reverseFeed)
                return ((list.headerItem && list.headerItem.visible) ? list.headerItem.height : scrollToTopButtonMargin) + 10
            else
                return parent.height - height - ((!ignoreFooter && list.footerItem && list.footerItem.visible) ? list.footerItem.height : scrollToTopButtonMargin) - 10
        }
    }

    Timer {
        id: hideScrollToTopButtonTimer
        interval: 2000

        onTriggered: scrollTopTopButtonVisible = false
    }

    onVerticalOvershootChanged: {
        if (!enabled)
            return

        if (topOvershootFun && !list.flicking && verticalOvershoot < (!reverseFeed ? -refreshText.height - 30 : 0))  {
            inTopOvershoot = true
        } else if (verticalOvershoot >= 0) {
            if (inTopOvershoot && !inProgress)
                topOvershootFun()

            inTopOvershoot = false
        }

        if (bottomOvershootFun && !list.flicking && verticalOvershoot > (reverseFeed ? refreshText.height + 30 : 0)) {
            inBottomOvershoot = true;
        } else {
            if (inBottomOvershoot && !inProgress)
                bottomOvershootFun()

            inBottomOvershoot = false;
        }
    }

    function showScrollToTopButton() {
        hideScrollToTopButtonTimer.stop()
        scrollTopTopButtonVisible = true
    }

    function hideScrollToTopButton() {
        hideScrollToTopButtonTimer.start()
    }


    Component.onDestruction: {
        if (Boolean(list)) {
            if (typeof list.onMovementStarted != 'undefined')
                list.onMovementStarted.disconnect(showScrollToTopButton)

            if (typeof list.onMovementEnded != 'undefined')
                list.onMovementEnded.disconnect(hideScrollToTopButton)
        }
    }

    Component.onCompleted: {
        list.onMovementStarted.connect(showScrollToTopButton)
        list.onMovementEnded.connect(hideScrollToTopButton)
    }
}
