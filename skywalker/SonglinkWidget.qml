import QtQuick
import QtQuick.Controls
import skywalker

Loader {
    required property bool showWidget
    required property string uri

    id: songlinkLoader
    width: parent.width
    active: showWidget && songlink.isMusicLink(uri)

    sourceComponent: Rectangle {
        width: parent.width
        height: (songlinkRow.visible ? songlinkRow.height : songlinkFlow.height + songlinkAttribution.height) + 10
        color: "transparent"

        Rectangle {
            id: songlinkRow
            anchors.centerIn: parent
            width: 36
            height: width
            radius: width / 2
            color: guiSettings.buttonNeutralColor
            visible: !songlinkFlow.songlinkQueryDone

            Image {
                anchors.centerIn: parent
                width: parent.width - 4
                height: width
                fillMode: Image.PreserveAspectFit
                source: guiSettings.isLightMode ? "/images/songlink-odesli-logo-light.png" : "/images/songlink-odesli-logo-dark.png"

                BusyIndicator {
                    anchors.centerIn: parent
                    width: parent.width
                    height: width
                    running: songlink.inProgress
                }
            }
        }

        Flow {
            property bool songlinkQueryDone: false
            property songlinklinks links
            readonly property int logoWidth: 50

            id: songlinkFlow
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width, links.linkInfoList.length * logoWidth + (links.linkInfoList.length - 1) * spacing + 2 * padding)
            spacing: 10
            padding: 5
            visible: songlinkQueryDone && !links.isNull()

            Repeater {
                model: songlinkFlow.links.linkInfoList

                Rectangle {
                    required property songlinkinfo modelData

                    id: linkItem
                    width: songlinkFlow.logoWidth
                    height: itemCol.height
                    color: "transparent"

                    Column {
                        id: itemCol
                        width: parent.width

                        Image {
                            x: 5
                            width: parent.width - 10
                            height: width
                            fillMode: Image.PreserveAspectFit
                            source: linkItem.modelData.logo
                        }

                        AccessibleText {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            font.pointSize: guiSettings.scaledFont(5/8)
                            wrapMode: Text.WordWrap
                            text: linkItem.modelData.name
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.openLink(linkItem.modelData.link)
                    }
                }
            }
        }

        AccessibleText {
            id: linksNotFound
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: guiSettings.scaledFont(5/8)
            text: qsTr("Not found on other streaming platforms.")
            visible: songlinkFlow.songlinkQueryDone && songlinkFlow.links.isNull()
        }

        AccessibleText {
            id: songlinkAttribution
            anchors.top: songlinkFlow.visible ? songlinkFlow.bottom : linksNotFound.bottom
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: guiSettings.scaledFont(5/8)
            text: "Powered by Songlink/Odesli"
            visible: songlinkFlow.songlinkQueryDone
        }

        function showMusicLinks(links) {
            songlinkFlow.links = links
            songlinkFlow.songlinkQueryDone = true
        }

        MouseArea {
            anchors.fill: parent
            enabled: !songlink.inProgress && songlinkRow.visible
            onClicked: songlink.getLinks(uri)
        }
    }

    Songlink {
        id: songlink

        onFailure: (error) => root.getSkywalker().showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onLinksFound: (links) => {
            if (songlinkLoader.item)
                songlinkLoader.item.showMusicLinks(links)
        }
    }
}
