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
    property int verticalOvershoot: list.verticalOvershoot
    property var scrollToTopFun: () => list.positionViewAtBeginning()
    property bool scrollTopTopButtonVisible: false
    property int scrollToTopButtonMargin: 0
    property bool enableScrollToTop: true
    property bool ignoreFooter: false

    anchors.fill: parent
    //z: parent.z - 1

    Text {
        id: refreshText
        anchors.horizontalCenter: parent.horizontalCenter
        y: ((list.headerItem && list.headerItem.visible) ? list.headerItem.height : 0) - verticalOvershoot - height
        font.italic: true
        color: guiSettings.textColor
        visible: verticalOvershoot < 0
        text: inTopOvershoot ? qsTr("Refreshing...") : topText
    }

    SvgButton {
        id: scrollToTopButton
        x: 7
        y: parent.height - height - ((!ignoreFooter && list.footerItem && list.footerItem.visible) ? list.footerItem.height : scrollToTopButtonMargin) - 10
        width: 50
        height: width
        iconColor: guiSettings.textColor
        Material.background: guiSettings.buttonNeutralColor
        svg: SvgOutline.scrollToTop
        accessibleName: qsTr("Scroll to top")
        visible: scrollTopTopButtonVisible && !list.atYBeginning && enableScrollToTop
        onClicked: scrollToTopFun()
    }

    Timer {
        id: hideScrollToTopButtonTimer
        interval: 2000

        onTriggered: scrollTopTopButtonVisible = false
    }

    onVerticalOvershootChanged: {
        if (!enabled)
            return

        if (topOvershootFun && verticalOvershoot < -refreshText.height - 10)  {
            inTopOvershoot = true
        } else if (verticalOvershoot >= 0) {
            if (inTopOvershoot && !inProgress)
                topOvershootFun()

            inTopOvershoot = false
        }

        if (bottomOvershootFun && verticalOvershoot > 0) {
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
        list.onMovementStarted.disconnect(showScrollToTopButton)
        list.onMovementEnded.disconnect(hideScrollToTopButton)
    }

    Component.onCompleted: {
        list.onMovementStarted.connect(showScrollToTopButton)
        list.onMovementEnded.connect(hideScrollToTopButton)
    }
}
