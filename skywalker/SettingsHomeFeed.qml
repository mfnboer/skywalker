import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ColumnLayout {
    required property var userPrefs
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    property string userDid: userSettings.getActiveUserDid()

    HeaderText {
        Layout.topMargin: 10
        Layout.bottomMargin: 10
        text: qsTr("Following feed preferences")
    }

    AccessibleCheckBox {
        text: qsTr("Show replies")
        checked: !userPrefs.hideReplies
        onCheckedChanged: userPrefs.hideReplies = !checked
    }

    AccessibleCheckBox {
        text: qsTr("Replies in threads from followed users only")
        checked: userSettings.getHideRepliesInThreadFromUnfollowed(userDid)
        enabled: !userPrefs.hideReplies
        onCheckedChanged: userSettings.setHideRepliesInThreadFromUnfollowed(userDid, checked)
    }

    AccessibleCheckBox {
        text: qsTr("Replies to followed users only")
        checked: userPrefs.hideRepliesByUnfollowed
        enabled: !userPrefs.hideReplies
        onCheckedChanged: userPrefs.hideRepliesByUnfollowed = checked
    }

    AccessibleCheckBox {
        text: qsTr("Assemble post threads")
        checked: userSettings.getAssembleThreads(userDid)
        enabled: !userPrefs.hideReplies
        onCheckedChanged: userSettings.setAssembleThreads(userDid, checked)
    }

    AccessibleCheckBox {
        id: showRepostsSwitch
        text: qsTr("Show reposts")
        checked: !userPrefs.hideReposts
        onCheckedChanged: userPrefs.hideReposts = !checked
    }

    AccessibleCheckBox {
        text: qsTr("Show reposted posts from followed users")
        checked: userSettings.getShowFollowedReposts(userDid)
        onCheckedChanged: userSettings.setShowFollowedReposts(userDid, checked)
        enabled: showRepostsSwitch.checked
    }

    AccessibleCheckBox {
        text: qsTr("Show self-reposts")
        checked: userSettings.getShowSelfReposts(userDid)
        onCheckedChanged: userSettings.setShowSelfReposts(userDid, checked)
        enabled: showRepostsSwitch.checked
    }

    AccessibleCheckBox {
        id: showQuotesSwitch
        text: qsTr("Show quote posts")
        checked: !userPrefs.hideQuotePosts
        onCheckedChanged: userPrefs.hideQuotePosts = !checked
    }

    AccessibleCheckBox {
        text: qsTr("Show quotes with blocked post")
        checked: userSettings.getShowQuotesWithBlockedPost(userDid)
        onCheckedChanged: userSettings.setShowQuotesWithBlockedPost(userDid, checked)
        enabled: showQuotesSwitch.checked
    }

    AccessibleCheckBox {
        text: qsTr("Rewind to last seen post at startup")
        checked: userSettings.getRewindToLastSeenPost(userDid)
        onCheckedChanged: userSettings.setRewindToLastSeenPost(userDid, checked)
    }
}
