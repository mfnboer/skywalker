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
                required property tenorcategory modelData
                property alias category: categoryEntry.modelData

                id: categoryEntry
                width: categoriesView.cellWidth
                height: categoriesView.cellHeight
                color: "transparent"

                AnimatedImage {
                    anchors.centerIn: parent
                    width: parent.width - 10
                    height: parent.height - 10
                    fillMode: Image.PreserveAspectCrop
                    source: category.gifUrl
                    cache: true

                    Text {
                        anchors.centerIn: parent
                        font.bold: true
                        color: "white"
                        text: category.searchTerm
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
