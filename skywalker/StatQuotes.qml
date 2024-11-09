import QtQuick
import skywalker

Text {
    required property string atUri
    required property int count

    color: GuiSettings.linkColor
    textFormat: Text.StyledText
    text: count > 1 ? qsTr(`<b>${count}</b> quotes`) : qsTr(`<b>${count}</b> quote`)
    visible: count

    Accessible.role: Accessible.Link
    Accessible.name: unicodeFonts.toPlainText(text)
    Accessible.onPressAction: showQuotes()

    MouseArea {
        anchors.fill: parent
        onClicked: parent.showQuotes()
    }

    function showQuotes() {
        root.viewQuotePostFeed(atUri)
    }

    UnicodeFonts {
        id: unicodeFonts
    }

}
