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

    AccessibleSwitch {
        text: qsTr("Show replies")
        checked: !userPrefs.hideReplies
        onCheckedChanged: userPrefs.hideReplies = !checked
    }

    AccessibleSwitch {
        text: qsTr("Replies in threads from followed users only")
        checked: userSettings.getHideRepliesInThreadFromUnfollowed(userDid)
        enabled: !userPrefs.hideReplies
        onCheckedChanged: userSettings.setHideRepliesInThreadFromUnfollowed(userDid, checked)
    }

    AccessibleSwitch {
        text: qsTr("Replies to followed users only")
        checked: userPrefs.hideRepliesByUnfollowed
        enabled: !userPrefs.hideReplies
        onCheckedChanged: userPrefs.hideRepliesByUnfollowed = checked
    }

    AccessibleSwitch {
        text: qsTr("Assemble post threads")
        checked: userSettings.getAssembleThreads(userDid)
        enabled: !userPrefs.hideReplies
        onCheckedChanged: userSettings.setAssembleThreads(userDid, checked)
    }

    AccessibleSwitch {
        id: showRepostsSwitch
        text: qsTr("Show reposts")
        checked: !userPrefs.hideReposts
        onCheckedChanged: userPrefs.hideReposts = !checked
    }

    AccessibleSwitch {
        text: qsTr("Show self-reposts")
        checked: userSettings.getShowSelfReposts(userDid)
        onCheckedChanged: userSettings.setShowSelfReposts(userDid, checked)
        enabled: showRepostsSwitch.checked
    }

    AccessibleSwitch {
        id: showQuotesSwitch
        text: qsTr("Show quote posts")
        checked: !userPrefs.hideQuotePosts
        onCheckedChanged: userPrefs.hideQuotePosts = !checked
    }

    AccessibleSwitch {
        text: qsTr("Show quotes with blocked post")
        checked: userSettings.getShowQuotesWithBlockedPost(userDid)
        onCheckedChanged: userSettings.setShowQuotesWithBlockedPost(userDid, checked)
        enabled: showQuotesSwitch.checked
    }

    AccessibleSwitch {
        text: qsTr("Rewind to last seen post at startup")
        checked: userSettings.getRewindToLastSeenPost(userDid)
        onCheckedChanged: userSettings.setRewindToLastSeenPost(userDid, checked)
    }
}
