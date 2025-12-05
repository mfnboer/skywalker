import QtQuick
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property generatorview feed
    property bool showCloseButton: false

    signal closeClicked

    id: quoteColumn
    padding: 10

    Accessible.role: Accessible.StaticText
    Accessible.name: accessibilityUtils.getFeedSpeech(feed)

    RowLayout {
        width: parent.width - 20

        FeedAvatar {
            id: avatar
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            Layout.alignment: Qt.AlignTop
            userDid: quoteColumn.userDid
            avatarUrl: feed.avatarThumb
            contentMode: feed.contentMode
            unknownSvg: guiSettings.feedDefaultAvatar(feed)

            onClicked: skywalker.getFeedGenerator(feed.uri)
        }

        Column {
            Layout.fillWidth: true

            SkyCleanedTextLine {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: feed.displayName

                Accessible.ignored: true
            }

            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: qsTr(`feed by @${feed.creator.handle}`)

                Accessible.ignored: true
            }
        }

        SvgButton {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            svg: SvgOutline.close
            accessibleName: qsTr("remove quoted feed")
            focusPolicy: Qt.NoFocus
            visible: showCloseButton
            onClicked: closeClicked()
        }
    }

    AccessibleText {
        width: parent.width - 20
        wrapMode: Text.Wrap
        maximumLineCount: 5
        elide: Text.ElideRight
        text: feed.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

}
