import QtQuick
import skywalker

// Should be used inside a Text element
SkyMouseArea {
    property string userDid
    property string containingText
    property basicprofile author

    signal longPress
    signal unrollThread

    anchors.fill: parent
    propagateComposedEvents: true

    onClicked: (mouse) => {
        const link = parent.linkAt(mouse.x, mouse.y)

        if (link) {
            if (link === UnicodeFonts.THREAD_LINK)
                unrollThread()
            else
                root.openLink(link, containingText, userDid)
        }
        else {
            mouse.accepted = false
        }
    }

    onPressAndHold: (mouse) => {
        const link = parent.linkAt(mouse.x, mouse.y)

        if (link) {
            if (UnicodeFonts.isHashtag(link))
                hashtagContextMenuLoader.activate(link)
            else
                longPress()
        }
        else {
            longPress()
        }
    }

    Loader {
        property string hashtag: ""

        id: hashtagContextMenuLoader
        active: false

        sourceComponent: HashtagContextMenu {
            onDone: hashtagContextMenuLoader.active = false
        }

        onStatusChanged: {
            if (status == Loader.Ready)
                item.show(hashtag, author.handle)
        }

        function activate(hashtag) {
            hashtagContextMenuLoader.hashtag = hashtag
            active = true
        }
    }
}
