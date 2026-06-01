import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    property string userDid
    property string uri
    property string title
    property bool titleIsHtml: false
    property string description
    property bool descriptionIsHtml: false
    property string thumbUrl
    property date createdAt
    property date updatedAt
    readonly property date documentDate: isNaN(updatedAt.getTime()) ? createdAt : updatedAt
    property int readingTime: 0 // minutes
    property externalsource externalSource
    property list<basicprofile> associatedProfiles: []
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property basicprofile contentLabeler
    property string borderColor: guiSettings.isLightMode ? Qt.darker(color, 1.1) : Qt.lighter(color, 1.6)
    property int columnHeight: externalColumn.height
    property bool showSonglinkWidget: false
    property bool isLiveExternal: false
    property date liveExpiresAt
    property bool moving: false

    id: card
    height: columnHeight
    cornerRadius: guiSettings.radius

    Rectangle {
        id: cardBackground
        anchors.fill: parent
        color: guiSettings.highLightColor(maskColor)
    }

    Column {
        id: externalColumn
        width: parent.width
        spacing: 3

        FilteredImageWarning {
            id: filter
            width: parent.width
            contentVisibility: card.contentVisibility
            contentWarning: card.contentWarning
            contentLabeler: card.contentLabeler
            imageUrl: card.thumbUrl
        }
        Item {
            width: parent.width
            height: visible ? 30 : 0
            visible: isLiveExternal & !card.thumbUrl
        }
        Loader {
            readonly property real aspectRatio: imageUtils.getPreferredLinkCardAspectRatio(card.uri)
            id: imgLoader
            x: (externalColumn.width - width) / 2
            width: calcWidth()
            height: aspectRatio * width
            active: filter.imageVisible() && Boolean(card.thumbUrl) && !moving
            asynchronous: true

            sourceComponent: ThumbImageFixedSizeView {
                width: imgLoader.width
                height: imgLoader.height
                canvasColor: cardBackground.color
                dynamicCanvasColor: false
                fillMode: Image.PreserveAspectCrop
                image: imageUtils.createImageView(filter.imageVisible() ? card.thumbUrl : "", "")
            }

            onStatusChanged: {
                if (status == Loader.Ready)
                    active = true
            }

            LoaderCanvas {
                backgroundColor: cardBackground.color
            }

            function calcWidth() {
                if (!card.thumbUrl)
                    return 0

                let w = externalColumn.width
                const h = aspectRatio * w

                if (h > guiSettings.maxImageHeight)
                    w *= (guiSettings.maxImageHeight / h)

                return w
            }
        }
        SkyCleanedText {
            id: titleText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            plainText: card.title
            wrapMode: Text.Wrap
            maximumLineCount: 3
            textFormat: titleIsHtml ? Text.RichText : Text.PlainText
            elide: titleIsHtml ? Text.ElideNone : Text.ElideRight
            font.bold: true
        }
        SkyCleanedText {
            id: descriptionText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            bottomPadding: 5
            plainText: card.description ? card.description : card.uri
            wrapMode: Text.Wrap
            maximumLineCount: 8
            textFormat: descriptionIsHtml ? Text.RichText : Text.PlainText
            elide: descriptionIsHtml ? Text.ElideNone : Text.ElideRight
        }
        Loader {
            active: !isNaN(card.documentDate.getTime()) || card.readingTime > 0
            sourceComponent: Row {
                x: 5
                width: externalColumn.width - 10
                spacing: 10

                AccessibleText {
                    color: guiSettings.messageTimeColor
                    font.pointSize: guiSettings.scaledFont(7/8)
                    text: !isNaN(card.documentDate.getTime()) ? card.documentDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat) : ""
                    visible: !isNaN(card.documentDate.getTime())
                }

                AccessibleText {
                    color: guiSettings.messageTimeColor
                    font.pointSize: guiSettings.scaledFont(7/8)
                    text: `🕓${card.readingTime}` + qsTr("m", "minutes")
                    visible: card.readingTime > 0
                }
            }
        }
        Loader {
            active: associatedProfiles.length > 0
            sourceComponent: SkyCleanedText {
                x: 5
                width: externalColumn.width - 10
                font.italic: true
                wrapMode: Text.Wrap
                plainText: qsTr(`by ${guiSettings.toAuthorNameSequence(associatedProfiles)}`)
            }
        }
        Loader {
            active: !externalSource.isNull()
            sourceComponent: Column {
                width: externalColumn.width

                Item {
                    width: parent.width
                    height: 5
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: borderColor
                }

                LinkCardSource {
                    width: parent.width
                    userDid: card.userDid
                    externalSource: card.externalSource
                    isCardLink: root.getLinkUtils(userDid).equalLinks(card.uri, card.externalSource.uri)
                }
            }
        }

        AccessibleText {
            id: linkText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            bottomPadding: 5
            text: card.uri ? new URL(card.uri).hostname : ""
            elide: Text.ElideRight
            color: guiSettings.linkColor
            visible: externalSource.isNull()
        }

        SonglinkWidget {
            showWidget: showSonglinkWidget
            uri: card.uri
        }

        Loader {
            anchors.horizontalCenter: parent.horizontalCenter
            active: isLiveExternal && !isNaN(liveExpiresAt.getTime())

            sourceComponent: AccessibleText {
                font.italic: true
                text: qsTr(`Till ${guiSettings.expiresIndication(liveExpiresAt)}`)
            }
        }

        Loader {
            anchors.horizontalCenter: parent.horizontalCenter
            active: isLiveExternal

            sourceComponent: SkyButton {
                implicitHeight: 40
                text: qsTr("Watch now")
                onClicked: root.openLink(card.uri, "", userDid)
            }
        }
    }

    Rectangle {
        anchors.fill: parent

        // HACK: without -1 the contents just peep out at the bottom ??
        anchors.bottomMargin: -1

        border.width: 1
        border.color: borderColor
        radius: cornerRadius
        color: "transparent"
    }

    ImageUtils {
        id: imageUtils
    }

    function getFilter() {
        return filter
    }
}
