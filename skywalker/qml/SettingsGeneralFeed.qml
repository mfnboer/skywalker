import QtQuick
import QtQuick.Layouts
import skywalker

ColumnLayout {
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()
    property string userDid: userSettings.getActiveUserDid()
    property list<favoritefeedview> favorites: skywalker.favoriteFeeds.userOrderedPinnedFeeds
    readonly property int labelSize: parent.width / 3

    HeaderText {
        Layout.topMargin: 10
        Layout.bottomMargin: 10
        text: qsTr("General feed preferences")
    }

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        rowSpacing: 5

        AccessibleText {
            Layout.preferredWidth: labelSize
            wrapMode: Text.Wrap
            text: qsTr("Home feed")
        }

        SkyComboBox {
            Layout.fillWidth: true
            model: ListModel {
                Component.onCompleted: {
                    append({ "value": skywalker.favoriteFeeds.getHomeFeedKey(), "text": "Following" })

                    for (const favorite of favorites)
                        append({ "value": favorite.key, "text": favorite.nameWithSubTitle })
                }
            }
            currentValue: skywalker.favoriteFeeds.getHomeFeedUri()
            onCurrentValueChanged: userSettings.setHomeFeedUri(skywalker.getUserDid(), currentValue)
        }

        AccessibleText {
            Layout.preferredWidth: labelSize
            wrapMode: Text.Wrap
            text: qsTr("Show at startup")
        }

        SkyComboBox {
            Layout.fillWidth: true
            model: ListModel {
                ListElement { value: false; text: qsTr("Home feed") }
                ListElement { value: true; text: qsTr("Last viewed feed") }
            }
            currentValue: userSettings.getStartLastViewedFeed(skywalker.getUserDid())
            onCurrentValueChanged: userSettings.setStartLastViewedFeed(skywalker.getUserDid(), currentValue)
        }
    }

    AccessibleCheckBox {
        text: qsTr("Assemble post threads")
        checked: userSettings.getAssembleThreads(userDid)
        onCheckedChanged: userSettings.setAssembleThreads(userDid, checked)
    }

    AccessibleCheckBox {
        text: qsTr("Show quotes with blocked post")
        checked: userSettings.getShowQuotesWithBlockedPost(userDid)
        onCheckedChanged: userSettings.setShowQuotesWithBlockedPost(userDid, checked)
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Posts order:")
    }

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.globalFeedOrder === QEnums.FEED_ORDER_PER_FEED
        text: qsTr("Per feed setting")
        onCheckedChanged: {
            if (checked)
                userSettings.globalFeedOrder = QEnums.FEED_ORDER_PER_FEED
        }
    }

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.globalFeedOrder === QEnums.FEED_ORDER_NEW_TO_OLD
        text: qsTr("New to old")
        onCheckedChanged: {
            if (checked)
                userSettings.globalFeedOrder = QEnums.FEED_ORDER_NEW_TO_OLD
        }
    }

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.globalFeedOrder === QEnums.FEED_ORDER_OLD_TO_NEW
        text: qsTr("Old to new")
        onCheckedChanged: {
            if (checked)
                userSettings.globalFeedOrder = QEnums.FEED_ORDER_OLD_TO_NEW
        }
    }
}
