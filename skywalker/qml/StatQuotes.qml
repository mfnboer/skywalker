import QtQuick
import skywalker

AccessibleText {
    property string userDid
    required property string atUri
    required property int count

    color: guiSettings.linkColor
    textFormat: Text.StyledText
    text: count > 1 ? qsTr(`<b>${count}</b> quotes`) : qsTr(`<b>${count}</b> quote`)
    visible: count

    Accessible.role: Accessible.Link
    Accessible.name: UnicodeFonts.toPlainText(text)
    Accessible.onPressAction: showQuotes()

    MouseArea {
        anchors.fill: parent
        onClicked: parent.showQuotes()
    }

    function showQuotes() {
        root.viewQuotePostFeed(atUri, userDid)
    }


}
