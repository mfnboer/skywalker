import QtQuick
import skywalker

Flow {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property list<profilelinkview> links

    spacing: 5

    Repeater {
        model: links

        Rectangle {
            required property profilelinkview modelData

            id: linkRect
            width: link.width + (linkIcon.active ? link.height + 2 : 0) + 10
            height: link.height + 4
            radius: height / 2
            color: guiSettings.linkButtonColor

            Loader {
                id: linkIcon
                x: 5
                y: 4
                active: modelData.icon
                asynchronous: true

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
                        source: linkRect.modelData.icon
                        fillMode: Image.PreserveAspectFit
                        indicateLoading: false
                    }
                }
            }

            SkyLabel {
                id: link
                anchors.left: linkIcon.right
                anchors.leftMargin: linkIcon.active ? 2 : 0
                y: 2
                width: Math.min(implicitWidth, guiSettings.maxLinkButtonWidth)
                backgroundColor: "transparent"
                font.pointSize: guiSettings.linkButtonFontSize
                font.weight: Font.Medium
                elide: Text.ElideRight
                text: (modelData.icon ? "" : "🌐 ") + getDisplayText()
            }

            SkyMouseArea {
                anchors.fill: parent
                onClicked: root.openLink(modelData.url, getDisplayText())
            }

            function getDisplayText() {
                return modelData.title ? modelData.title : modelData.url
            }
        }
    }
}
