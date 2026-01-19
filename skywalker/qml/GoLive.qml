import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string uri
    property int duration: 60

    id: dialog
    width: parent.width - 20
    contentHeight: col.height
    title: qsTr("Go live")
    modal: true
    standardButtons: linkCard.visible && isDurationValid() ? Dialog.Ok | Dialog.Cancel : Dialog.Cancel
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    Column {
        id: col
        width: parent.width
        spacing: 10

        AccessibleText {
            width: parent.width
            wrapMode: Text.Wrap
            text: qsTr("Add a temporary live status to your profile. Information about your live event will be shown on your profile.")
        }

        SkyWebLinkInput {
            id: websiteField
            width: parent.width
            placeholderText: qsTr("Live link")
            text: uri

            onTextChanged: {
                if (valid)
                    linkCardTimer.start()
                else
                    linkCardTimer.stop()
            }
        }

        RowLayout {
            id: durationRow
            width: parent.width

            Text {
                text: qsTr("Duration:")
            }
            SkyTumbler {
                id: hoursField
                model: 8
                currentIndex: Math.floor(duration / 60)
                background: Rectangle {
                    color: isDurationValid() ? guiSettings.textInputBackgroundColor : guiSettings.textInputInvalidColor
                }
            }
            Text {
                text: qsTr("hours")
            }
            SkyTumbler {
                id: minutesField
                model: 60
                currentIndex: duration % 60
                background: Rectangle {
                    color: isDurationValid() ? guiSettings.textInputBackgroundColor : guiSettings.textInputInvalidColor
                }
            }

            AccessibleText {
                text: qsTr("minutes")
            }

            Item {
                Layout.fillWidth: true
            }
        }

        LinkCardView {
            property var card: null

            id: linkCard
            width: parent.width
            uri: card ? card.link : ""
            title: card ? card.title : ""
            description: card ? card.description : ""
            thumbUrl: card ? card.thumb : ""
            contentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
            contentWarning: ""
            visible: card

            function show(card) {
                linkCard.card = card
            }

            function hide() {
                linkCard.card = null
            }
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: linkCardReader.inProgress
    }

    LinkCardReader {
        property bool inProgress: false

        id: linkCardReader

        onLinkCard: (card) => {
            inProgress = false
            console.debug("Got card:", card.link, card.title, card.thumb)
            console.debug(card.description)
            linkCard.show(card)
        }

        onLinkCardFailed: {
            inProgress= false
            console.debug("Failed to get link card")
            linkCard.hide()
        }

        function getCard(link) {
            inProgress = true
            getLinkCard(link)
        }
    }

    Timer {
        id: linkCardTimer
        interval: 1000
        onTriggered: linkCardReader.getCard(websiteField.displayText)
    }

    function getLinkCard() {
        return linkCard.card
    }

    function getDuration() {
        const duration = hoursField.currentIndex * 60 + minutesField.currentIndex
        return duration
    }

    function isDurationValid() {
        return getDuration() > 0
    }
}
