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
    property bool showSonglinkWidget: false
    property bool isLiveExternal: false

    id: card
    height: columnHeight
    cornerRadius: 10
    border.width: 1
    border.color: borderColor

    Column {
        id: externalColumn
        width: parent.width
        topPadding: 1
        spacing: 3

        FilteredImageWarning {
            id: filter
            width: parent.width - 2
            contentVisibility: card.contentVisibility
            contentWarning: card.contentWarning
            imageUrl: card.thumbUrl
        }
        Loader {
            active: filter.imageVisible() && Boolean(card.thumbUrl)
            width: parent.width

            sourceComponent: ThumbImageUnknownSizeView {
                x: (parent.width - width) / 2
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

        SonglinkWidget {
            showWidget: showSonglinkWidget
            uri: card.uri
        }

        Loader {
            anchors.horizontalCenter: parent.horizontalCenter
            active: isLiveExternal

            sourceComponent: SkyButton {
                text: qsTr("Watch now")
                onClicked: root.openLink(card.uri)
            }
        }
    }

    ImageUtils {
        id: imageUtils
    }
}
