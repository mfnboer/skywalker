import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

// TODO: follows-you-label, follow/unfollow onClicked
ListView {
    required property string title
    required property var skywalker
    required property int modelId
    property int margin: 8

    property bool inBottomOvershoot: false

    signal closed

    id: authorListView
    spacing: 0
    model: skywalker.getAuthorListModel(modelId)
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            id: headerRow
            width: parent.width
            height: guiSettings.headerHeight

            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: title
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: AuthorViewDelegate {
        viewWidth: authorListView.width
    }

    onVerticalOvershootChanged: {
        if (verticalOvershoot > 0) {
            if (!inBottomOvershoot && !skywalker.getAuthorListInProgress) {
                skywalker.getAuthorListNextPage(modelId)
            }

            inBottomOvershoot = true;
        } else {
            inBottomOvershoot = false;
        }
    }

    BusyIndicator {
        id: busyBottomIndicator
        anchors.centerIn: parent
        running: skywalker.getAuthorListInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        skywalker.removeAuthorListModel(modelId)
    }
}
