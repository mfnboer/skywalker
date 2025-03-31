import QtQuick

// SHould be used inside a Text element
MouseArea {
    property string containingText

    anchors.fill: parent

    onClicked: (mouse) => {
        const link = parent.linkAt(mouse.x, mouse.y)

        if (link)
            root.openLink(link, containingText)
    }

    onPressAndHold: (mouse) => {
        const link = parent.linkAt(mouse.x, mouse.y)

        if (link) {
            if (UnicodeFonts.isHashtag(link))
                hashtagContextMenuLoader.activate(link)
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

    function showHashtagMenu() {

    }
}
