import QtQuick
import QtQuick.Controls

Container {
    id: view

    property real minPageWidth: 360
    property int maxColumns: 10
    property bool interactive: true
    readonly property int columns: Math.min(maxColumns, Math.max(1, Math.floor(width / minPageWidth)))
    readonly property real pageWidth: width / columns
    readonly property int firstVisibleIndex: currentIndex
    readonly property int lastVisibleIndex: currentIndex + columns - 1

    background: Rectangle {
        color: guiSettings.separatorColor
    }

    contentItem: ListView {
        model: view.contentModel
        spacing: view.spacing
        currentIndex: view.currentIndex
        orientation: ListView.Horizontal
        snapMode: ListView.SnapToItem
        boundsBehavior: Flickable.StopAtBounds
        interactive: view.interactive
        highlightRangeMode: ListView.StrictlyEnforceRange
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        clip: true

        onCurrentIndexChanged: view.setCurrentIndex(currentIndex)
    }

    // Size the pages
    onPageWidthChanged: relayout()
    onCountChanged: relayout()

    function relayout() {
        for (let i = 0; i < count; ++i) {
            let item = itemAt(i)
            item.width = pageWidth
            item.height = Qt.binding(() => view.height)
        }
    }

    function isVisibleIndex(index) {
        return index >= firstVisibleIndex && index <= lastVisibleIndex
    }
}