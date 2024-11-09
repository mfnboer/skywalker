import QtQuick
import QtQuick.Layouts
import skywalker

Column {
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
            width: 34
            Layout.alignment: Qt.AlignTop
            avatarUrl: feed.avatarThumb

            onClicked: skywalker.getFeedGenerator(feed.uri)
        }

        Column {
            Layout.fillWidth: true

            SkyCleanedTextLine {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: GuiSettings.textColor
                plainText: feed.displayName

                Accessible.ignored: true
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: GuiSettings.scaledFont(7/8)
                color: GuiSettings.handleColor
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

    Text {
        width: parent.width - 20
        wrapMode: Text.Wrap
        maximumLineCount: 5
        elide: Text.ElideRight
        color: GuiSettings.textColor
        text: feed.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

}
