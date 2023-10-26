import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    property var timeline

    signal closed

    id: page

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            width: parent.width
            height: guiSettings.headerHeight

            SvgButton {
                id: backButton
                iconColor: "white"
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: page.closed()
            }

            Rectangle {
                radius: 5
                Layout.fillWidth: true
                height: searchText.height
                color: guiSettigs.backgroundColor

                TextInput {
                    id: searchText
                    width: parent.width
                    padding: 5
                    font.pointSize: guiSettings.scaledFont(9/8)

                    onTextChanged: {
                        if (length > 0)
                            typeaheadSearchTimer.start()
                        else
                            typeaheadSearchTimer.stop()
                    }
                }

                Text {
                    width: searchText.width
                    padding: searchText.padding
                    font.pointSize: searchText.font.pointSize
                    color: "grey"
                    text: qsTr("Search")
                    visible: searchText.length === 0
                }
            }

            Rectangle {
                width: 10
                height: parent.height
                color: "transparent"
            }
        }
    }

    footer: SkyFooter {
        timeline: page.timeline
        skywalker: page.skywalker
        searchActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: root.viewNotifications()
    }

    AuthorTypeaheadListView {
        anchors.fill: parent
        model: searchUtils.authorTypeaheadList
    }

    Timer {
        id: typeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (searchText.length > 0)
                searchUtils.searchAuthorsTypeahead(searchText.text)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker
    }

    GuiSettings {
        id: guiSettigs
    }
}
