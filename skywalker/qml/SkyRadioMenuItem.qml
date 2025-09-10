import QtQuick

AccessibleMenuItem {
    checkable: true

    Component.onCompleted: {
        indicator.radius = indicator.height / 2
    }
}
