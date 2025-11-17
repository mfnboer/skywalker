import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ListView {
    required property Skywalker skywalker
    readonly property string sideBarTitle: qsTr("Muted words")
    readonly property string sideBarSubTitle: `${view.count} / ${skywalker.mutedWords.maxSize}`
    readonly property SvgImage sideBarSvg: SvgOutline.mutedWords

    signal closed

    id: view
    spacing: 0
    boundsBehavior: Flickable.StopAtBounds
    model: skywalker.mutedWords.entries
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    header: SimpleHeader {
        text: sideBarTitle
        subTitle: sideBarSubTitle
        headerVisible: !root.showSideBar
        onBack: view.closed()

        SvgPlainButton {
            anchors.top: parent.top
            anchors.right: parent.right
            svg: SvgOutline.add
            onClicked: addWord()
            accessibleName: qsTr(`add word to mute`)
            visible: view.count < skywalker.mutedWords.maxSize
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: ColumnLayout {
        required property mutedwordentry modelData

        width: view.width

        RowLayout {
            Layout.fillWidth: true

            MutedWordEntry {
                id: mutedWordEntry
                Layout.fillWidth: true
                entry: modelData
            }
            SvgPlainButton {
                svg: SvgOutline.edit
                accessibleName: qsTr(`edit ${mutedWordEntry.text}`)
                onClicked: editWord(modelData)
            }
            SvgPlainButton {
                svg: SvgOutline.delete
                accessibleName: qsTr(`delete ${mutedWordEntry.text}`)
                onClicked: skywalker.mutedWords.removeEntry(modelData.value)
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
        svg: SvgOutline.mutedWords
        text: qsTr("No muted words")
        list: view
    }

    function addWord() {
        let component = guiSettings.createComponent("AddMutedWord.qml")
        let dialog = component.createObject(view)

        dialog.onAccepted.connect(() => {
            const word = dialog.getText()
            const actorTarget = dialog.excludeFollows ? QEnums.ACTOR_TARGET_EXCLUDE_FOLLOWING : QEnums.ACTOR_TARGET_ALL

            if (word)
                skywalker.mutedWords.addEntry(word, actorTarget, dialog.expiresAt)

            dialog.destroy()
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.show()
    }

    function editWord(mutedWordEntry) {
        let component = guiSettings.createComponent("AddMutedWord.qml")
        let dialog = component.createObject(view, {
                editWord: mutedWordEntry.value,
                expiresAt: mutedWordEntry.expiresAt,
                excludeFollows: mutedWordEntry.actorTarget === QEnums.ACTOR_TARGET_EXCLUDE_FOLLOWING
            })

        dialog.onAccepted.connect(() => {
            const word = dialog.getText()
            const actorTarget = dialog.excludeFollows ? QEnums.ACTOR_TARGET_EXCLUDE_FOLLOWING : QEnums.ACTOR_TARGET_ALL

            if (word && (word !== mutedWordEntry.value ||
                actorTarget !== mutedWordEntry.actorTarget ||
                dialog.expiresAt !== mutedWordEntry.expiresAt))
            {
                skywalker.mutedWords.removeEntry(mutedWordEntry.value)
                skywalker.mutedWords.addEntry(word, actorTarget, dialog.expiresAt)
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
