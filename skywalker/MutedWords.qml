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
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    header: SimpleHeader {
        text: qsTr("Muted words") + ` (${view.count} / ${skywalker.mutedWords.maxSize})`
        onBack: view.closed()

        SvgButton {
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
            AccessibleText {
                id: entryText
                Layout.fillWidth: true
                leftPadding: 10
                rightPadding: 10
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                font.pointSize: guiSettings.scaledFont(9/8)
                text: modelData.value
            }
            SvgButton {
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                svg: SvgOutline.edit
                accessibleName: qsTr(`edit ${entryText.text}`)
                onClicked: editWord(entryText.text)
            }
            SvgButton {
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                svg: SvgOutline.delete
                accessibleName: qsTr(`delete ${entryText.text}`)
                onClicked: skywalker.mutedWords.removeEntry(entryText.text)
            }
        }

        AccessibleText {
            Layout.fillWidth: true
            leftPadding: 10
            rightPadding: 10
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: Material.color(Material.Grey)
            text: getExpiresIndication(modelData.expiresAt)
            visible: !isNaN(modelData.expiresAt.getTime())
        }

        AccessibleText {
            Layout.fillWidth: true
            leftPadding: 10
            rightPadding: 10
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: Material.color(Material.Grey)
            text: qsTr("Exclude users you follow")
            visible: modelData.actorTarget === QEnums.ACTOR_TARGET_EXCLUDE_FOLLOWING
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

    function getExpiresIndication(expiresAt) {
        if (isNaN(expiresAt))
            return ""

        if (guiSettings.isToday(expiresAt))
            return qsTr(`Expires ${Qt.locale().toString(expiresAt, Qt.locale().timeFormat(Locale.ShortFormat))}`)
        else if (guiSettings.isTomorrow(expiresAt))
            return qsTr(`Expires tomorrow ${Qt.locale().toString(expiresAt, Qt.locale().timeFormat(Locale.ShortFormat))}`)
        else if (expiresAt < new Date())
            return "Expired"
        else
            return qsTr(`Expires ${expiresAt.toLocaleString(Qt.locale(), Locale.ShortFormat)}`)
    }

    function addWord() {
        let component = Qt.createComponent("AddMutedWord.qml")
        let dialog = component.createObject(view)

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
        let dialog = component.createObject(view, { editWord: oldWord })

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
