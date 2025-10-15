import QtQuick
import QtQuick.Controls
import skywalker

Popup {
    required property int mouseY // in parent coordinates
    required property string postUri
    property string title
    property Skywalker skywalker: root.getSkywalker()
    property SessionManager sessionManager: skywalker.getSessionManager()

    signal userClicked(NonActiveUser user)

    id: popup
    x: 10
    width: parent.width - 20
    modal: true

    NonActiveUserListView {
        id: view
        anchors.fill: parent
        model: sessionManager.nonActiveNotifications
        postUri: popup.postUri

        onContentHeightChanged: {
            setPosition()
        }

        onUserClicked: (user) => {
            if (user.sessionExpired)
                skywalker.showStatusMessage(qsTr(`@${user.profile.handle} not logged in`), QEnums.STATUS_LEVEL_ERROR)
            else if (!Boolean(user.postView) || !user.postView.isGood())
                skywalker.showStatusMessage(qsTr(`@${user.profile.handle}: TODO ERROR`), QEnums.STATUS_LEVEL_ERROR)
            else
                popup.userClicked(user)
        }

        header: Rectangle {
            width: parent.width
            height: 34
            color: guiSettings.backgroundColor

            AccessibleText {
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.right: closeButton.left
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                font.bold: true
                elide: Text.ElideRight
                text: popup.title
            }

            SvgButton {
                id: closeButton
                anchors.rightMargin: 5
                anchors.right: parent.right
                width: height
                height: parent.height
                svg: SvgOutline.close
                accessibleName: qsTr("close")
                onClicked: popup.destroy()
            }
        }
    }

    function setPosition() {
        const belowHeight = parent?.height - mouseY - guiSettings.footerHeight

        if (belowHeight > 200) {
            y = mouseY
            height = Math.min(belowHeight, view.contentHeight + popup.topPadding + popup.bottomPadding)
            return
        }

        const aboveHeight = mouseY - guiSettings.headerHeight
        height = Math.min(aboveHeight, view.contentHeight + popup.topPadding + popup.bottomPadding)
        y = mouseY - height
    }

    Component.onCompleted: {
        setPosition()
    }
}
