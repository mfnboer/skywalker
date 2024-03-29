import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    signal closed
    signal selected(tenorgif gif)

    id: page
    clip: true

    Accessible.role: Accessible.Pane

    header: SearchHeader {
        minSearchTextLength: 2
        placeHolderText: qsTr("Search Tenor")

        onBack: {
            if (!viewStack.isCategoriesShowing())
                viewStack.showCategories()
            else
                page.closed()
        }

        onSearch: (text) => searchTenor(text)
    }

    footer: Rectangle {
        width: parent.width
        height: guiSettings.footerHeight
        z: guiSettings.footerZLevel
        color: guiSettings.backgroundColor

        Image {
            id: tenorAttribution
            anchors.fill: parent
            anchors.margins: 10
            fillMode: Image.PreserveAspectFit
            source: "/images/PB_tenor_logo_blue_horizontal.svg"
        }
    }

    StackLayout {
        id: viewStack
        anchors.fill: parent

        // Categories
        GridView {
            id: categoriesView
            width: parent.width
            height: parent.height
            cellWidth: width / 2
            cellHeight: 140
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

                    onWidthChanged: imgLabel.adjustWidth()

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr(`GIF category: ${category.searchTerm}`)
                    Accessible.onPressAction: searchTenor(category.searchTerm)

                    Label {
                        id: imgLabel
                        anchors.centerIn: parent
                        leftPadding: 5
                        rightPadding: 5
                        background: Rectangle { color: "black"; opacity: 0.2; radius: 5 }
                        elide: Text.ElideRight
                        font.bold: true
                        font.pointSize: guiSettings.scaledFont(9/8)
                        color: "white"
                        text: category.searchTerm

                        onWidthChanged: adjustWidth()

                        Accessible.ignored: true

                        function adjustWidth() {
                            if (width > parent.width)
                                width = parent.width
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: searchTenor(category.searchTerm)
                    }
                }
            }
        }

        // GIFs
        ListView {
            id: gifOverview
            width: parent.width
            model: tenor.overviewModel
            spacing: tenor.spacing
            clip: true
            flickDeceleration: guiSettings.flickDeceleration

            delegate: Row {
                required property list<tenorgif> previewRow
                required property int previewRowSpacing

                spacing: previewRowSpacing
                width: gifOverview.width

                Repeater {
                    model: previewRow.length

                    AnimatedImage {
                        required property int index
                        property tenorgif gif: previewRow[index]

                        id: gifDisplay
                        width: gif.overviewSize.width
                        height: gif.overviewSize.height
                        fillMode: Image.Stretch
                        source: gif.smallUrl

                        Accessible.role: Accessible.Button
                        Accessible.name: `GIF: ${gif.description}`
                        Accessible.onPressAction: selected(gif)

                        MouseArea {
                            anchors.fill: parent
                            onClicked: selected(gif)
                        }
                    }
                }
            }

            FlickableRefresher {
                inProgress: tenor.searchInProgress
                verticalOvershoot: gifOverview.verticalOvershoot
                bottomOvershootFun: () => tenor.getNextPage()
                topText: ""
            }
        }

        function isCategoriesShowing() {
            return currentIndex === 0;
        }

        function showCategories() {
            currentIndex = 0
        }

        function showGifs() {
            currentIndex = 1
        }
    }

    Tenor {
        id: tenor
        width: parent.width
        spacing: 4

        onCategories: (categoryList) => categoriesView.model = categoryList
        onSearchGifsFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: tenor.searchInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    function searchTenor(text) {
        tenor.searchGifs(text)
        viewStack.showGifs()
    }

    Component.onDestruction: {
        page.header.unfocus()
    }

    Component.onCompleted: {
        tenor.getCategories()
        page.header.unfocus()
    }
}
