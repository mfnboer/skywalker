import QtQuick

Connections {
    property int fullPageHeight: 0

    target: Qt.inputMethod

    // Resize the page when the Android virtual keyboard is shown
    function onKeyboardRectangleChanged() {
        if (Qt.inputMethod.keyboardRectangle.y > 0) { // qmllint disable missing-property
            // Sometimes the page height gets changed automatically but most times not...
            // Setting to to keyboard-y seems reliable.
            const keyboardY = Qt.inputMethod.keyboardRectangle.y  / Screen.devicePixelRatio
            parent.height = keyboardY
        }
        else {
            console.debug("HIDE KEYBOARD, PARENT:", parent.height, "CONTENT:", contentHeight)

            if (fullPageHeight > 0)
                parent.height = fullPageHeight
            else
                console.warn("fullPageHeight not set!")
        }
    }

    Component.onDestruction: {
        if (fullPageHeight > 0)
            parent.height = fullPageHeight
    }
}

