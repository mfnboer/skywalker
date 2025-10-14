import QtQuick
import QtQuick.Controls
import skywalker

Popup {
    required property int mouseY // in parent coordinates
    property string title
    property Skywalker skywalker: root.getSkywalker()
    property SessionManager sessionManager: skywalker.getSessionManager()
    property var nonActiveNotifications: sessionManager.nonActiveNotifications

    id: popup
    x: 10
    width: parent.width - 20
    modal: true

    SimpleAuthorListView {
        id: view
        anchors.fill: parent
        model: getNonActiveProfiles()

        onContentHeightChanged: {
            setPosition()
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
                font.pointSize: guiSettings.scaledFont(10/8)
                elide: Text.ElideRight
                text: popup.title
            }

            SvgButton {
                id: closeButton
                anchors.rightMargin: 10
                anchors.right: parent.right
                width: height
                height: parent.height
                svg: SvgOutline.close
                accessibleName: qsTr("close")
                onClicked: popup.destroy()
            }
        }
    }

    function getNonActiveProfiles() {
        let profiles = []

        for (const user of nonActiveNotifications)
            profiles.push(user.profile)

        return profiles
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
