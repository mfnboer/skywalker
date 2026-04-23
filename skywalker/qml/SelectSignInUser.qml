import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property list<basicprofile> userList
    readonly property string sideBarTitle: qsTr("Sign in as")

    signal selectedUser(basicprofile user)
    signal deletedUser(basicprofile user)
    signal canceled

    id: page

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: canceled()
    }

    SimpleAuthorListView {
        anchors.fill: parent
        rowPadding: 13
        allowDelete: true
        model: userList
        showHost: true
        onAuthorClicked: (profile) => { selectedUser(profile) }
        onAuthorPressAndHold: (profile) => { askResetUser(profile) }
        onDeleteClicked: (profile) => { deleteUser(profile) }
    }

    function askResetUser(profile) {
        guiSettings.askYesNoQuestion(
                    page,
                    qsTr(`Do you want to reset the session of account "${profile.name}"?`),
                    () => page.resetUser(profile))
    }

    function resetUser(profile) {
        let skywalker = root.getSkywalker()
        let userSettings = skywalker.getUserSettings()
        userSettings.clearTokens(profile.did)
        skywalker.showStatusMessage(qsTr("Session reset"), QEnums.STATUS_LEVEL_INFO)
    }

    function deleteUser(profile) {
        guiSettings.askYesNoQuestion(
                    page,
                    qsTr(`Do you really want to delete account "${profile.name}"?`),
                    () => page.deletedUser(profile))
    }

    function closed() {
        canceled()
    }
}
