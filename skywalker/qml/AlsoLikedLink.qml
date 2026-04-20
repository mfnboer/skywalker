import QtQuick

AccessibleText {
    property string userDid
    required property string postUri

    topPadding: 10
    bottomPadding: 5
    color: guiSettings.linkColor
    text: qsTr("also liked →")

    SkyMouseArea {
        anchors.fill: parent
        onClicked: root.viewAlsoLikedPostFeed(postUri, userDid)
    }
}
