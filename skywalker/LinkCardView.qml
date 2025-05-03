import QtQuick
import QtQuick.Controls.Material
import skywalker

RoundCornerMask {
    property string uri
    property string title
    property string description
    property string thumbUrl
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property string borderColor: guiSettings.borderColor
    property int columnHeight: externalColumn.height

    id: card
    height: columnHeight
    cornerRadius: 10
    border.width: 1
    border.color: borderColor

    FilteredImageWarning {
        id: filter
        width: parent.width - 2
        contentVisibility: card.contentVisibility
        contentWarning: card.contentWarning
        imageUrl: card.thumbUrl
    }

    Column {
        id: externalColumn
        width: parent.width
        topPadding: 1
        spacing: 3

        // HACK: The filter should be in this place, but inside a rounded object links
        // cannot be clicked.
        Rectangle {
            width: filter.width
            height: filter.height
            color: "transparent"
        }
        Loader {
            active: filter.imageVisible() && Boolean(card.thumbUrl)
            width: parent.width

            sourceComponent: ThumbImageUnknownSizeView {
                x: 1
                maxWidth: parent.width - 2
                maxHeight: guiSettings.maxImageHeight
                image: imageUtils.createImageView(filter.imageVisible() ? card.thumbUrl : "", "")
                noCrop: true
                indicateLoading: false

                onStatusChanged: {
                    if (status === Image.Error)
                        height = 0
                }
            }
        }
        Text {
            id: linkText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            text: card.uri ? new URL(card.uri).hostname : ""
            elide: Text.ElideRight
            color: guiSettings.linkColor
        }
        Text {
            id: titleText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            color: Material.foreground
            text: card.title
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            font.bold: true
        }
        Text {
            id: descriptionText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            bottomPadding: 5
            color: Material.foreground
            text: card.description ? card.description : card.uri
            wrapMode: Text.Wrap
            maximumLineCount: 5
            elide: Text.ElideRight
        }
        Loader {
            id: songlinkLoader
            width: parent.width
            active: songlink.isMusicLink(card.uri)

            sourceComponent: Rectangle {
                width: parent.width
                height: (songlinkRow.visible ? songlinkRow.height : songlinkFlow.height + songlinkAttribution.height) + 10
                color: "transparent"

                Rectangle {
                    id: songlinkRow
                    anchors.centerIn: parent
                    width: 40
                    height: width
                    radius: width / 2
                    color: guiSettings.buttonNeutralColor
                    visible: !songlinkFlow.visible

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
                    property songlinklinks links
                    readonly property int logoWidth: 50

                    id: songlinkFlow
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(parent.width, links.linkInfoList.length * logoWidth + (links.linkInfoList.length - 1) * spacing + 2 * padding)
                    spacing: 10
                    padding: 5
                    visible: !links.isNull()

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
                    id: songlinkAttribution
                    anchors.top: songlinkFlow.bottom
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: guiSettings.scaledFont(5/8)
                    text: "Powered by Songlink/Odesli"
                    visible: songlinkFlow.visible
                }

                function showMusicLinks(links) {
                    songlinkFlow.links = links
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: !songlink.inProgress && songlinkRow.visible
                    onClicked: songlink.getLinks(card.uri)
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
    }

    ImageUtils {
        id: imageUtils
    }
}
