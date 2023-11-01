import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Drawer {
    property list<basicprofile> userList

    signal selectedUser(basicprofile user)

    id: drawer

    AuthorTypeaheadListView {
        anchors.fill: parent
        rowPadding: 13
        model: userList
        onAuthorClicked: (profile) => { selectedUser(profile) }

        header: Text {
            anchors.horizontalCenter: parent.horizontalCenter
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            text: qsTr("Switch User")
        }
    }

    SvgButton {
        id: closeButton
        anchors.right: parent.right
        iconColor: guiSettings.textColor
        Material.background: "transparent"
        svg: svgOutline.close
        onClicked: drawer.close()
    }

    GuiSettings {
        id: guiSettings
    }
}
