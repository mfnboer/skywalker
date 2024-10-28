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
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    header: SimpleHeader {
        text: qsTr("Focus hashtags") + ` (${view.count} / ${skywalker.focusHashtags.maxSize})`
        onBack: view.closed()

        SvgButton {
            anchors.right: parent.right
            svg: SvgOutline.add
            onClicked: addHashtagEntry()
            accessibleName: qsTr(`add hashtag for focus`)
            visible: view.count < skywalker.focusHashtags.maxSize
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: Column {
        required property var modelData // FocusHashtagEntry

        width: view.width
        spacing: 0

        Rectangle {
            width: parent.width
            height: hashtagRow.height
            color: "transparent"

            RowLayout {
                id: hashtagRow
                width: parent.width
                spacing: 0

                AccessibleText {
                    id: entryText
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight
                    padding: 10
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                    font.pointSize: guiSettings.scaledFont(9/8)
                    color: guiSettings.textColor
                    text: getEntryText(modelData)

                    onLinkActivated: (hashtag) => {
                        hashtagMenu.selectedTag = hashtag
                        hashtagMenu.open()
                    }

                    Menu {
                        property string selectedTag

                        id: hashtagMenu
                        modal: true

                        onAboutToShow: root.enablePopupShield(true)
                        onAboutToHide: root.enablePopupShield(false)

                        CloseMenuItem {
                            text: qsTr("<b>Hashtag</b>")
                            Accessible.name: qsTr("close hashtag menu")
                        }
                        AccessibleMenuItem {
                            text: qsTr("Edit")
                            onTriggered: editHashtagInEntry(modelData, hashtagMenu.selectedTag)
                            MenuItemSvg { svg: SvgOutline.edit }
                        }
                        AccessibleMenuItem {
                            text: qsTr("Delete")
                            onTriggered: removeHashtagFromEntry(modelData, hashtagMenu.selectedTag)
                            MenuItemSvg { svg: SvgOutline.delete }
                        }
                    }
                }
                SvgButton {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    Layout.rightMargin: 5
                    imageMargin: 0
                    iconColor: guiSettings.textColor
                    Material.background: "transparent"
                    svg: SvgOutline.palette
                    accessibleName: qsTr(`set hightlight color for ${entryText.text}`)
                    onClicked: setHighlightColor(modelData)
                }
                SvgButton {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    Layout.rightMargin: 5
                    imageMargin: 0
                    iconColor: guiSettings.textColor
                    Material.background: "transparent"
                    svg: SvgOutline.add
                    accessibleName: qsTr(`edit ${entryText.text}`)
                    onClicked: addHashtagToEntry(modelData)
                }
                SvgButton {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    Layout.rightMargin: 10
                    imageMargin: 0
                    id: deleteButton
                    iconColor: guiSettings.textColor
                    Material.background: "transparent"
                    svg: SvgOutline.delete
                    accessibleName: qsTr(`delete ${entryText.text}`)
                    onClicked: deleteHashtagEntry(modelData)
                }
            }

            Rectangle {
                anchors.fill: parent
                z: parent.z - 1
                color: modelData.highlightColor
                opacity: guiSettings.focusHighlightOpacity
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: guiSettings.separatorColor
        }
    }

    FlickableRefresher {}

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.hashtag
        text: qsTr("No focus hashtags")
        list: view
    }

    GuiSettings {
        id: guiSettings
    }

    function deleteHashtagEntry(entry) {
        guiSettings.askYesNoQuestion(
                    view,
                    qsTr(`Do you really want to delete: #${entry.hashtags[0]} ?`),
                    () => skywalker.focusHashtags.removeEntry(entry.id))
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
                skywalker.focusHashtags.addHashtagToEntry(entry, tag)

            dialog.destroy()
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.show()
    }

    function removeHashtagFromEntry(entry, hashtag) {
        skywalker.focusHashtags.removeHashtagFromEntry(entry, hashtag)
    }

    function editHashtagInEntry(entry, hashtag) {
        let component = Qt.createComponent("AddFocusHashtag.qml")
        let dialog = component.createObject(view, { focusHashtag: hashtag });

        dialog.onAccepted.connect(() => {
            const tag = dialog.getText()

            if (tag) {
                if (entry.size() > 1) {
                    skywalker.focusHashtags.removeHashtagFromEntry(entry, hashtag)
                    skywalker.focusHashtags.addHashtagToEntry(entry, tag)
                }
                else {
                    skywalker.focusHashtags.addEntry(tag, entry.highlightColor)
                    skywalker.focusHashtags.removeEntry(entry.id)
                }
            }

            dialog.destroy()
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.show()
    }

    function setHighlightColor(entry) {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(view)
        cs.selectedColor = entry.highlightColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            entry.highlightColor = cs.selectedColor
            cs.destroy()
        })
        cs.open()
    }

    function getEntryText(entry) {
        let text = ""

        for (let i = 0; i < entry.hashtags.length; ++i) {
            const tag = entry.hashtags[i]

            if (i > 0)
                text += ' '

            text += `<a href="${tag}" style="color: ${guiSettings.linkColor}; text-decoration: none">#${tag}</a>`
        }

        console.debug("TEXT:", text)
        return text
    }

    Component.onDestruction: {
        skywalker.focusHashtags.save(skywalker.getUserDid(), skywalker.getUserSettings())
    }
}
