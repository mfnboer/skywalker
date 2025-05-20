import QtQuick

Row {
    property bool followedBy: false
    property string blockingUri
    property bool blockingByList: false
    property bool blockedBy: false
    property bool muted: false
    property bool mutedByList: false
    property bool mutedReposts: false
    property bool hideFromTimeline: false

    spacing: 5

    SkyLabel {
        text: qsTr("follows you")
        visible: followedBy
    }
    SkyLabel {
        text: getBlockingText()
        visible: blockingUri && !blockingByList
    }
    SkyLabel {
        text: qsTr("blocks you")
        visible: blockedBy
    }
    SkyLabel {
        text: qsTr("list blocked")
        visible: blockingByList
    }
    SkyLabel {
        text: qsTr("muted")
        visible: muted && !mutedByList
    }
    SkyLabel {
        text: qsTr("list muted")
        visible: mutedByList
    }
    SkyLabel {
        text: qsTr("muted reposts")
        visible: mutedReposts
    }
    SkyLabel {
        text: qsTr("hide from timeline")
        visible: hideFromTimeline
    }

    function getBlockingText() {
        const blocksWithExpiry = root.getSkywalker().getUserSettings().blocksWithExpiry
        const expiresAt = blocksWithExpiry.getExpiry(blockingUri)

        if (!isNaN(expiresAt.getTime()))
            return qsTr(`blocked till ${expiresIndication(expiresAt)}`)

        return qsTr("blocked")
    }
}
