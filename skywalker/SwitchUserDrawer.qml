import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Drawer {
    property list<basicprofile> userList

    signal selectedUser(basicprofile user)

    id: drawer

    SimpleAuthorListView {
        anchors.fill: parent
        rowPadding: 9
        model: userList
        onAuthorClicked: (profile) => { selectedUser(profile) }

        header: Rectangle {
            height: GuiSettings.headerHeight
            width: parent.width
            z: GuiSettings.headerZLevel
            color: "transparent"

            AccessibleText {
                anchors.centerIn: parent
                font.bold: true
                font.pointSize: GuiSettings.scaledFont(10/8)
                color: GuiSettings.textColor
                text: qsTr("Switch Account")
            }

            SvgButton {
                id: closeButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                svg: SvgOutline.close
                accessibleName: qsTr("cancel switch user")
                onClicked: drawer.close()
            }
        }
        headerPositioning: ListView.OverlayHeader
    }

}
