import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    property list<basicprofile> userList

    signal selectedUser(basicprofile user)
    signal deletedUser(basicprofile user)
    signal canceled

    id: page

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            width: parent.width
            height: guiSettings.headerHeight

            SvgButton {
                id: backButton
                iconColor: "white"
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: page.canceled()
            }
            Text {
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: qsTr("Sign in as")
            }
        }
    }

    SimpleAuthorListView {
        anchors.fill: parent
        rowPadding: 13
        allowDelete: true
        model: userList
        onAuthorClicked: (profile) => { selectedUser(profile) }
        onDeleteClicked: (profile) => { deleteUser(profile) }
    }

    GuiSettings {
        id: guiSettings
    }

    function deleteUser(profile) {
        guiSettings.askYesNoQuestion(
                    page,
                    qsTr(`Do you really want to delete account "${profile.name}"?`),
                    () => page.deletedUser(profile))
    }
}
