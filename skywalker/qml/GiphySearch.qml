import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    readonly property string sideBarTitle: qsTr("Add GIF")
    readonly property SvgImage sideBarSvg: SvgOutline.addGif

    signal closed
    signal selected(tenorgif gif)

    id: page
    clip: true

    Accessible.role: Accessible.Pane

    header: SearchHeader {
        minSearchTextLength: 2
        placeHolderText: qsTr("Search Giphy")
        showBackButton: !root.showSideBar

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
            x: 10
            y: 10
            width: parent.width - 20
            height: parent.height - 20
            fillMode: Image.PreserveAspectFit
            source: "/images/PB_tenor_logo_blue_horizontal.svg" // TODO
            asynchronous: true
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
            maximumFlickVelocity: guiSettings.maxFlickVelocity
            pixelAligned: guiSettings.flickPixelAligned

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
                    Accessible.onPressAction: searchCategory(category)

                    AccessibleLabel {
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

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: searchCategory(category)
                    }
                }
            }

            FlickableRefresher {}
        }

        // GIFs
        ListView {
            id: gifOverview
            width: parent.width
            model: giphy.overviewModel
            spacing: giphy.spacing
            clip: true
            flickDeceleration: guiSettings.flickDeceleration
            maximumFlickVelocity: guiSettings.maxFlickVelocity
            pixelAligned: guiSettings.flickPixelAligned

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

                        SkyMouseArea {
                            anchors.fill: parent
                            onClicked: {
                                giphy.addRecentGif(gif)
                                selected(gif)
                            }
                        }
                    }
                }
            }

            FlickableRefresher {
                inProgress: giphy.searchInProgress
                bottomOvershootFun: () => giphy.getNextPage()
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

    Giphy {
        id: giphy
        width: parent.width
        spacing: 4
        skywalker: root.getSkywalker()

        onCategories: (categoryList) => categoriesView.model = categoryList
        onSearchGifsFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: giphy.searchInProgress
    }


    function searchTenor(text) {
        giphy.searchGifs(text)
        viewStack.showGifs()
    }

    function searchCategory(category) {
        if (category.isRecentCategory) {
            giphy.searchRecentGifs()
            viewStack.showGifs()
        }
        else {
            searchTenor(category.searchTerm)
        }
    }

    Component.onDestruction: {
        page.header.unfocus()
    }

    Component.onCompleted: {
        giphy.getCategories()
        page.header.unfocus()
    }
}
