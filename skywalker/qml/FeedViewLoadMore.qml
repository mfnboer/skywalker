import QtQuick
import skywalker

Rectangle {
    required property var listView
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property string textColor: guiSettings.textColor
    property string linkColor: guiSettings.linkColor

    id: item
    width: listView.width
    height: 150
    color: "transparent"

    AccessibleText {
        y: listView.model && listView.model.reverseFeed && listView.count > 0 ? parent.height - height : 0
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        padding: 10
        textFormat: Text.RichText
        wrapMode: Text.Wrap
        color: item.textColor
        text: qsTr(`${guiSettings.getFilteredPostsFooterText(listView.model)}<br><a href="load" style="color: ${item.linkColor}; text-decoration: none">Load more</a>`)
        onLinkActivated: listView.model.getFeedNextPage(skywalker)
    }
}
