import QtQuick

// Should be used inside a Text element
MouseArea {
    property string containingText

    signal longPress

    anchors.fill: parent
    propagateComposedEvents: true

    onClicked: (mouse) => {
        const link = parent.linkAt(mouse.x, mouse.y)

        if (link)
            root.openLink(link, containingText)
        else
            mouse.accepted = false
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
                item.show(hashtag)
        }

        function activate(hashtag) {
            hashtagContextMenuLoader.hashtag = hashtag
            active = true
        }
    }
}
