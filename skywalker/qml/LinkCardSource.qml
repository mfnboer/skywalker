import QtQuick
import skywalker

Rectangle {
    property string userDid
    required property externalsource externalSource
    property bool isCardLink: false
    readonly property string standardSitePublisher: externalSource.getStandardSitePublisher()
    readonly property SvgImage standardSitePublisherIcon: externalSource.getStandardSitePublisherIcon()

    height: Math.max(iconFrame.height, sourceColumn.height) + 10
    color: getBackgroundColor()

    RoundedFrame {
        id: iconFrame
        x: 5
        y: 5
        objectToRound: iconImg.status === Image.Ready ? iconImg : null
        width: 60
        height: width
        radius: guiSettings.radius
        visible: externalSource.icon && iconImg.status === Image.Ready

        ImageAutoRetry {
            id: iconImg
            anchors.fill: parent
            source: externalSource.icon
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
            fillMode: Image.PreserveAspectFit
            maxRetry: 60
            indicateLoading: false
            visible: false // for rounding
        }
    }

    Column {
        id: sourceColumn
        y: 5 + Math.max((iconFrame.height - sourceColumn.height) / 2, 0)
        anchors.left: externalSource.icon ? iconFrame.right : parent.left
        anchors.leftMargin: externalSource.icon ? 10 : 5
        anchors.right: parent.right
        anchors.rightMargin: 5

        SkyCleanedTextLine {
            width: parent.width
            plainText: externalSource.title
            elide: Text.ElideRight
            color: getForegroundColor()
            font.bold: true
            visible: !isCardLink
        }

        SkyCleanedText {
            width: parent.width
            plainText: externalSource.description
            maximumLineCount: 1
            elide: Text.ElideRight
            color: getForegroundColor()
            visible: !isCardLink && externalSource.description
        }

        Row {
            width: parent.width
            spacing: 5

            SkySvg {
                id: icon
                width: guiSettings.appFontHeight + 4
                height: width
                svg: standardSitePublisherIcon
                color: getAccentColor()
            }

            AccessibleText {
                y: 2
                width: parent.width - icon.width - parent.spacing
                color: getAccentColor()
                text: standardSitePublisher ? qsTr(`Subscribe on ${standardSitePublisher}`) :
                                              guiSettings.stripHttpFromLink(externalSource.uri)
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.openLink(externalSource.uri, "", userDid)
    }

    function getBackgroundColor() {
        if (externalSource.theme.isNull() || !externalSource.theme.backgroundRGB.valid)
            return "transparent"

        return externalSource.theme.backgroundRGB
    }

    function getForegroundColor() {
        if (externalSource.theme.isNull() || !externalSource.theme.foregroundRGB.valid)
            return guiSettings.textColor

        return externalSource.theme.foregroundRGB
    }

    function getAccentColor() {
        if (externalSource.theme.isNull() || !externalSource.theme.accentRGB.valid)
            return guiSettings.textColor

        return externalSource.theme.accentRGB
    }
}
