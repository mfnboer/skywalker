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
            height: guiSettings.headerHeight
            width: parent.width
            z: guiSettings.headerZLevel
            color: "transparent"

            AccessibleText {
                anchors.centerIn: parent
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: guiSettings.textColor
                text: qsTr("Switch Account")
            }

            SvgButton {
                id: closeButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                svg: svgOutline.close
                onClicked: drawer.close()

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("cancel switch user")
                Accessible.onPressAction: clicked()
            }
        }
        headerPositioning: ListView.OverlayHeader
    }

    GuiSettings {
        id: guiSettings
    }
}
