import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    property string userDid
    required property basicprofile author

    id: knownOthersRow
    spacing: 10

    Row {
        id: avatarRow
        topPadding: 10
        spacing: -20
        visible: author.viewer.knownFollowers.count > 0

        Repeater {
            model: author.viewer.knownFollowers.followers

            Avatar {
                required property int index
                required property basicprofile modelData

                z: 5 - index
                width: 34
                userDid: knownOthersRow.userDid
                author: modelData
                showFollowingStatus: false
                onClicked: knownOthersRow.showKnownFollowers()
            }
        }
    }

    SkyCleanedText {
        id: knownFollowersText
        Layout.fillWidth: true
        topPadding: 10
        elide: Text.ElideRight
        wrapMode: Text.Wrap
        textFormat: Text.RichText
        maximumLineCount: 3
        color: guiSettings.linkColor
        plainText: qsTr(`Followed by ${getKnownFollowersText()}`)
        visible: author.viewer.knownFollowers.count > 0

        MouseArea {
            anchors.fill: parent
            onClicked: knownOthersRow.showKnownFollowers()
        }
    }

    AccessibleText {
        Layout.fillWidth: true
        topPadding: 10
        wrapMode: Text.Wrap
        font.italic: true
        text: qsTr("Not followed by anyone you follow")
        visible: author.viewer.knownFollowers.count === 0
    }

    function showKnownFollowers() {
        let modelId = root.getSkywalker(userDid).createAuthorListModel(QEnums.AUTHOR_LIST_KNOWN_FOLLOWERS, author.did)
        root.viewAuthorListByUser(userDid, modelId, qsTr(`Followers you follow`));
    }

    function getKnownFollowersText() {
        const knownFollowers = author.viewer.knownFollowers

        if (knownFollowers.count <= 0)
            return ""

        if (knownFollowers.followers.length === 0)
            return knownFollowers.count > 1 ? qsTr(`${knownFollowers.count} others you follow`) : qsTr("1 other you follow")

        let followersText = UnicodeFonts.toCleanedHtml(knownFollowers.followers[0].name)

        if (knownFollowers.followers.length > 1)
            followersText = `${followersText}, ${UnicodeFonts.toCleanedHtml(knownFollowers.followers[1].name)}`

        if (knownFollowers.count > 3)
            followersText = qsTr(`${followersText} and ${(knownFollowers.count - 2)} others`)
        else if (knownFollowers.count > 2)
            followersText = qsTr(`${followersText} and 1 other`)

        return followersText
    }
}
