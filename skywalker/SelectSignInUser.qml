import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property list<basicprofile> userList

    signal selectedUser(basicprofile user)
    signal deletedUser(basicprofile user)
    signal canceled

    id: page

    Accessible.role: Accessible.Pane

    header: Rectangle {
        width: parent.width
        height: GuiSettings.headerHeight
        z: GuiSettings.headerZLevel
        color: GuiSettings.headerColor

        Accessible.role: Accessible.Pane

        RowLayout {
            width: parent.width
            height: GuiSettings.headerHeight

            SvgButton {
                id: backButton
                iconColor: GuiSettings.headerTextColor
                Material.background: "transparent"
                svg: SvgOutline.arrowBack
                accessibleName: qsTr("go back")
                onClicked: page.canceled()
            }
            Text {
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 10
                font.bold: true
                font.pointSize: GuiSettings.scaledFont(10/8)
                color: GuiSettings.headerTextColor
                text: qsTr("Sign in as")

                Accessible.role: Accessible.TitleBar
                Accessible.name: qsTr("Select user to sign in")
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


    function deleteUser(profile) {
        GuiSettings.askYesNoQuestion(
                    page,
                    qsTr(`Do you really want to delete account "${profile.name}"?`),
                    () => page.deletedUser(profile))
    }
}
