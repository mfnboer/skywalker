import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    signal closed

    id: page

    header: SearchHeader {
        minSearchTextLength: 2
        placeHolderText: qsTr("Search Tenor")
        onBack: page.closed()
        onSearch: (text) => { tenor.searchGifs(text) }
    }

    StackLayout {
        anchors.fill: parent

        GridView {
            id: categoriesView
            width: parent.width
            height: parent.height
            cellWidth: width / 2
            cellHeight: 100
            model: []
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            flickDeceleration: guiSettings.flickDeceleration

            delegate: Rectangle {
                required property int index
                required property tenorcategory modelData
                property alias category: categoryEntry.modelData

                id: categoryEntry
                width: categoriesView.cellWidth
                height: categoriesView.cellHeight
                color: "transparent"

                AnimatedImage {
                    x: index & 1 ? 2 : 0
                    width: parent.width - 2
                    height: parent.height - 4
                    fillMode: Image.PreserveAspectCrop
                    source: category.gifUrl
                    cache: true

                    onWidthChanged: imgLabel.adjustWidth()

                    Label {
                        id: imgLabel
                        anchors.centerIn: parent
                        leftPadding: 5
                        rightPadding: 5
                        background: Rectangle { color: "black"; opacity: 0.2; radius: 5 }
                        elide: Text.ElideRight
                        font.bold: true
                        color: "white"
                        text: category.searchTerm

                        onWidthChanged: adjustWidth()

                        function adjustWidth() {
                            if (width > parent.width)
                                width = parent.width
                        }
                    }
                }
            }
        }
    }

    Tenor {
        id: tenor

        onCategories: (categoryList) => categoriesView.model = categoryList
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        tenor.getCategories()
        page.header.unfocus()
    }
}
