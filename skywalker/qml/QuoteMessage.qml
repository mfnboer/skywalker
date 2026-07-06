import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Column {
    required property convoview convo
    required property messageview message
    required property basicprofile author

    signal closeClicked

    id: quoteColumn

    RowLayout {
        width: parent.width - 10

        SkyCleanedTextLine {
            id: authorName
            Layout.fillWidth: true
            padding: 10
            // leftPadding: 10
            // rightPadding: 10
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.bold: true
            font.pointSize: guiSettings.scaledFont(7/8)
            plainText: quoteColumn.author.name
        }

        SvgButton {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            svg: SvgOutline.close
            accessibleName: qsTr("remove reply-to message")
            focusPolicy: Qt.NoFocus
            onClicked: closeClicked()
        }
    }

    Flickable {
        id: flick
        width: parent.width
        height: Math.min(contentHeight, guiSettings.appFontHeight * 5)
        clip: true
        contentHeight: contentItem.childrenRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: SkyScrollBarVertical {}

        MessageText {
            id: messageText
            width: parent.width - (height > guiSettings.appFontHeight * 5 ? 10 : 0)
            convo: quoteColumn.convo
            message: quoteColumn.message
            author: quoteColumn.author
            addEmbedToText: true
        }
    }

    Item {
        width: parent.width
        height: 10
    }
}
