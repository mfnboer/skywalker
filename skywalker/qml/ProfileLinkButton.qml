import QtQuick
import skywalker

Rectangle {
    required property profilelinkview linkView
    required property int visibility // QEnums::ContentVisibility

    signal clicked

    id: linkRect
    width: link.width + (linkIcon.active ? link.height + 2 : 0) + 10
    height: link.height + 4
    radius: height / 2
    color: guiSettings.linkButtonColor

    Loader {
        id: linkIcon
        x: 5
        y: 4
        active: linkRect.linkView.icon && linkRect.visibility === QEnums.CONTENT_VISIBILITY_SHOW
        asynchronous: true
        visible: status == Loader.Ready

        sourceComponent: RoundCornerMask {
            width: height
            height: link.height - 4
            cornerRadius: width * 0.1
            maskColor: linkRect.color

            ImageAutoRetry {
                id: iconImg
                anchors.fill: parent
                sourceSize.width: width * Screen.devicePixelRatio
                sourceSize.height: height * Screen.devicePixelRatio
                source: linkRect.linkView.icon
                fillMode: Image.PreserveAspectFit
                indicateLoading: false
            }
        }
    }

    SkyLabel {
        id: link
        anchors.left: linkIcon.active ? linkIcon.right : linkIcon.left
        anchors.leftMargin: linkIcon.active ? 2 : 0
        y: 2
        width: Math.min(implicitWidth, guiSettings.maxLinkButtonWidth)
        backgroundColor: "transparent"
        font.pointSize: guiSettings.linkButtonFontSize
        font.weight: Font.Medium
        elide: Text.ElideRight
        text: getDisplayIconPrefix() + getDisplayText()
    }

    SkyMouseArea {
        anchors.fill: parent
        enabled: parent.enabled
        onClicked: linkRect.clicked()
    }

    function getDisplayText() {
        return linkView.title ? linkView.title : guiSettings.toLinkDisplayText(linkView.url)
    }

    function getDisplayIconPrefix() {
        if (visibility === QEnums.CONTENT_VISIBILITY_SHOW)
            return linkView.icon ? "" : "🌐 "

        return "⚠️ "
    }
}