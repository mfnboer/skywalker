import QtQuick
import QtQuick.Controls
import skywalker

Popup {
    required property int mouseY // in parent coordinates
    required property string postUri
    required property int action // QEnums::NonActiveUserAction
    property Skywalker skywalker: root.getSkywalker()
    property SessionManager sessionManager: skywalker.getSessionManager()

    signal userClicked(NonActiveUser user)
    signal repostClicked(NonActiveUser user)
    signal quoteClicked(NonActiveUser user)

    id: popup
    x: 10
    width: parent.width - 20
    modal: true

    background: Rectangle {
        radius: 5
        color: guiSettings.backgroundColor
    }

    NonActiveUserListView {
        id: view
        anchors.fill: parent
        model: sessionManager.nonActiveNotifications
        postUri: popup.postUri
        action: popup.action

        onContentHeightChanged: {
            setPosition()
        }

        onUserClicked: (user) => {
            if (user.sessionExpired)
                skywalker.showStatusMessage(qsTr(`@${user.profile.handle} not logged in`), QEnums.STATUS_LEVEL_INFO)
            else if (!Boolean(user.postView))
                console.debug("No post view available")
            else if (user.postView.uri !== postUri)
                console.debug("Post not loaded:", postUri)
            else if (user.postView.hasError())
                skywalker.showStatusMessage(qsTr(`@${user.profile.handle}: ${user.postView.error}`), QEnums.STATUS_LEVEL_ERROR)
            else if (user.postView.notFound)
                skywalker.showStatusMessage(qsTr(`@${user.profile.handle}: post not available`), QEnums.STATUS_LEVEL_INFO)
            else
                popup.userClicked(user)
        }

        onRepostClicked: (user) => popup.repostClicked(user)
        onQuoteClicked: (user) => popup.quoteClicked(user)

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
                text: getTitle()
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

    function getTitle() {
        switch (action) {
        case QEnums.NON_ACTIVE_USER_LIKE:
            return qsTr("Like with")
        case QEnums.NON_ACTIVE_USER_BOOKMARK:
            return qsTr("Bookmark with")
        case QEnums.NON_ACTIVE_USER_REPLY:
            return qsTr("Reply with")
        case QEnums.NON_ACTIVE_USER_REPOST:
            return qsTr("Repost or quote with")
        }

        return ""
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

    Component.onDestruction: {
        sessionManager.startPostCacheTimeout()
    }

    Component.onCompleted: {
        setPosition()
        sessionManager.stopPostCacheTimeout()
    }
}
