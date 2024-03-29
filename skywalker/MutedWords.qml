import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker

    signal closed

    id: view
    spacing: 0
    boundsBehavior: Flickable.StopAtBounds
    model: skywalker.mutedWords.entries
    flickDeceleration: guiSettings.flickDeceleration
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    header: SimpleHeader {
        text: qsTr("Muted words") + ` (${view.count} / ${skywalker.mutedWords.maxSize})`
        onBack: view.closed()

        SvgButton {
            anchors.right: parent.right
            svg: svgOutline.add
            onClicked: addWord()
            visible: view.count < skywalker.mutedWords.maxSize

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`add word to mute`)
            Accessible.onPressAction: clicked()
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: ColumnLayout {
        required property string modelData

        width: view.width

        RowLayout {
            AccessibleText {
                id: entryText
                Layout.fillWidth: true
                leftPadding: 10
                rightPadding: 10
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                font.pointSize: guiSettings.scaledFont(9/8)
                color: guiSettings.textColor
                text: modelData
            }
            SvgButton {
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                svg: svgOutline.edit
                onClicked: editWord(entryText.text)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr(`edit ${entryText.text}`)
                Accessible.onPressAction: clicked()
            }
            SvgButton {
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                svg: svgOutline.delete
                onClicked: skywalker.mutedWords.removeEntry(entryText.text)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr(`delete ${entryText.text}`)
                Accessible.onPressAction: clicked()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.mutedWords
        text: qsTr("No muted words")
        list: view
    }

    GuiSettings {
        id: guiSettings
    }

    function addWord() {
        let component = Qt.createComponent("AddMutedWord.qml")
        let dialog = component.createObject(view);

        dialog.onAccepted.connect(() => {
            const word = dialog.getText()

            if (word)
                skywalker.mutedWords.addEntry(word)

            dialog.destroy()
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.show()
    }

    function editWord(oldWord) {
        let component = Qt.createComponent("AddMutedWord.qml")
        let dialog = component.createObject(view, { editWord: oldWord });

        dialog.onAccepted.connect(() => {
            const word = dialog.getText()

            if (word && word !== oldWord) {
                skywalker.mutedWords.removeEntry(oldWord)
                skywalker.mutedWords.addEntry(word)
            }

            dialog.destroy()
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.show()
    }

    Component.onDestruction: {
        skywalker.saveMutedWords()
    }
}
