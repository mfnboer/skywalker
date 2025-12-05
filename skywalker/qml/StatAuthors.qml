import QtQuick
import skywalker

AccessibleText {
    property string userDid
    required property string atUri
    required property int count
    required property string nameSingular
    required property string namePlural
    required property int authorListType // QEnums.AuthorListType
    required property string authorListHeader

    color: guiSettings.linkColor
    textFormat: Text.StyledText
    text: count > 1 ? qsTr(`<b>${count}</b> ${namePlural}`) : qsTr(`<b>${count}</b> ${nameSingular}`)
    visible: count

    Accessible.role: Accessible.Link
    Accessible.name: UnicodeFonts.toPlainText(text)
    Accessible.onPressAction: showAuthors()

    MouseArea {
        anchors.fill: parent
        onClicked: parent.showAuthors()
    }

    function showAuthors() {
        let modelId = root.getSkywalker(userDid).createAuthorListModel(authorListType, atUri)
        root.viewAuthorListByUser(userDid, modelId, authorListHeader);
    }


}
