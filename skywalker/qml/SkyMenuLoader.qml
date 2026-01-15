import QtQuick

// Helper for buttons that launch menus

// TODO update PostFeedHeader
// TODO maybe PostStats and AuthorView too.
Loader {
    active: false

    onStatusChanged: {
        if (status == Component.Ready)
            item.open()
    }

    function open() {
        if (!active)
            active = true
        else
            item.open()
    }
}
