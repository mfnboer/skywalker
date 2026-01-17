import QtQuick

// Helper for buttons that launch menus
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
