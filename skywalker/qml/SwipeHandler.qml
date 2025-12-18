import QtQuick

DragHandler {
    property real startY: 0

    signal swipeUp
    signal swipeDown

    target: null

    onActiveChanged: {
        if (active) {
            startY = centroid.position.y
        } else {
            const deltaY = centroid.position.y - startY

            if (deltaY > 90)
                swipeDown()
            else if (deltaY < -90)
                swipeUp()
        }
    }
}
