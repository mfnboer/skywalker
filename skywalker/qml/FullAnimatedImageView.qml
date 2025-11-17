import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property string imageUrl
    property string imageTitle
    readonly property bool noSideBar: true

    signal closed

    id: page
    width: parent.width
    height: parent.height
    padding: 10
    background: Rectangle { color: guiSettings.fullScreenColor }

    Text {
        id: altText
        anchors.left: parent.left
        anchors.leftMargin: guiSettings.leftMargin
        anchors.right: parent.right
        anchors.rightMargin: guiSettings.rightMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: guiSettings.footerMargin
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        color: "white"
        text: imageTitle
        visible: imageTitle
    }
    AnimatedImageAutoRetry {
        id: img
        y: (parent.height - altText.height - height) / 2
        width: parent.width
        height: parent.height - altText.height
        fillMode: Image.PreserveAspectFit
        source: imageUrl
    }
    BusyIndicator {
        id: gifLoadingIndicator
        anchors.centerIn: parent
        running: img.status === Image.Loading
    }

    SvgButton {
        x: guiSettings.leftMargin
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

    Component.onDestruction: {
        resetSystemBarsColor()
    }

    Component.onCompleted: {
        setSystemBarsColor()
    }
}
