import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    property var skywalker: root.getSkywalker()

    signal closed

    id: view
    spacing: 0
    boundsBehavior: Flickable.StopAtBounds
    model: skywalker.focusHashtags.entries
    flickDeceleration: guiSettings.flickDeceleration
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    header: SimpleHeader {
        text: qsTr("Focus hashtags") + ` (${view.count} / ${skywalker.focusHashtags.maxSize})`
        onBack: view.closed()

        SvgButton {
            anchors.right: parent.right
            svg: svgOutline.add
            onClicked: addHashtagEntry()
            accessibleName: qsTr(`add hashtag for focus`)
            visible: view.count < skywalker.focusHashtags.maxSize
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: ColumnLayout {
        required property var modelData // FocusHashtagEntry

        width: view.width

        RowLayout {
            spacing: 0

            AccessibleText {
                id: entryText
                Layout.fillWidth: true
                leftPadding: 10
                rightPadding: 10
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                font.pointSize: guiSettings.scaledFont(9/8)
                color: guiSettings.textColor
                text: modelData.hashtags.join(" ")
            }
            SvgButton {
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                svg: svgOutline.add
                accessibleName: qsTr(`edit ${entryText.text}`)
                onClicked: addHashtagToEntry(modelData)
            }
            SvgButton {
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                svg: svgOutline.delete
                accessibleName: qsTr(`delete ${entryText.text}`)
                onClicked: skywalker.focusHashtags.removeEntry(modelData.id)
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }
    }

    FlickableRefresher {}

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.hashtag
        text: qsTr("No focus hashtags")
        list: view
    }

    GuiSettings {
        id: guiSettings
    }

    function addHashtagEntry() {
        let component = Qt.createComponent("AddFocusHashtag.qml")
        let dialog = component.createObject(view);

        dialog.onAccepted.connect(() => {
            const tag = dialog.getText()

            if (tag)
                skywalker.focusHashtags.addEntry(tag)

            dialog.destroy()
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.show()
    }

    function addHashtagToEntry(entry) {
        let component = Qt.createComponent("AddFocusHashtag.qml")
        let dialog = component.createObject(view);

        dialog.onAccepted.connect(() => {
            const tag = dialog.getText()

            if (tag)
                entry.addHashtag(tag)

            dialog.destroy()
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.show()
    }
}
