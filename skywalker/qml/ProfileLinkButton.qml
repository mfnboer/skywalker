import QtQuick
import skywalker

Rectangle {
    required property profilelinkview linkView
    required property int visibility // QEnums::ContentVisibility

    signal clicked

    id: linkRect
    width: link.width + linkIcon.width + 2 + 20
    height: link.height + 4
    radius: height / 2
    color: guiSettings.linkButtonColor

    Item {
        id: linkIcon
        x: 10
        y: 4
        width: height
        height: link.height - 4

        Loader {
            active: linkRect.linkView.icon && linkRect.visibility === QEnums.CONTENT_VISIBILITY_SHOW
            asynchronous: true
            visible: status == Loader.Ready

            sourceComponent: RoundCornerMask {
                width: linkIcon.width
                height: linkIcon.height
                cornerRadius: width * 0.15
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

        Loader {
            anchors.centerIn: parent
            active: !linkRect.linkView.icon || linkRect.visibility !== QEnums.CONTENT_VISIBILITY_SHOW
            visible: status == Loader.Ready

            sourceComponent: AccessibleText {
                text: visibility === QEnums.CONTENT_VISIBILITY_SHOW ? "🌐" : "⚠️"
            }
        }
    }

    SkyLabel {
        id: link
        anchors.left: linkIcon.right
        anchors.leftMargin: 2
        y: 2
        width: Math.min(implicitWidth, guiSettings.maxLinkButtonWidth)
        backgroundColor: "transparent"
        font.pointSize: guiSettings.linkButtonFontSize
        font.weight: Font.Medium
        elide: Text.ElideRight
        text: getDisplayText()
    }

    SkyMouseArea {
        anchors.fill: parent
        enabled: parent.enabled
        onClicked: linkRect.clicked()
    }

    function getDisplayText() {
        return linkView.title ? linkView.title : guiSettings.toLinkDisplayText(linkView.url)
    }
}