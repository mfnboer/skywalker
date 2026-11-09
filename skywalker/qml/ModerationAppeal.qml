import QtQuick
import skywalker

SkyPage {
    required property string decision
    required property basicprofile moderationProfile
    property Skywalker skywalker: root.getSkywalker()
    readonly property int margin: 10

    signal closed
    signal sendAppeal(string reason)

    id: page
    width: parent.width
    topPadding: 10
    bottomPadding: 10

    header: SimpleHeader {
        text: qsTr("Appeal")
        backIsCancel: true
        onBack: page.closed()

        SkyButton {
            anchors.rightMargin: parent.usedRightMargin + 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Send")
            enabled: detailsText.graphemeLength > 0 && !detailsText.maxGraphemeLengthExceeded()
            onClicked: sendAppeal(detailsText.text)
        }
    }

    footer: Rectangle {
        id: pageFooter
        width: page.width
        height: guiSettings.footerHeight + (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : 0)
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor

        TextLengthBar {
            textField: detailsText
        }

        TextLengthCounter {
            y: 10
            anchors.rightMargin: page.margin
            anchors.right: parent.right
            textField: detailsText
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: detailsField.y + detailsField.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        AccessibleText {
            id: appealTitle
            width: parent.width
            leftPadding: page.margin
            rightPadding: page.margin
            wrapMode: Text.Wrap
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            text: decision
        }

        AccessibleText {
            id: appealHeaderText
            anchors.top: appealTitle.bottom
            width: parent.width
            leftPadding: page.margin
            rightPadding: page.margin
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr(`This appeal will be sent to ${getModerationHandle()}`)
        }

        Rectangle {
            id: detailsField
            anchors.top: appealHeaderText.bottom
            anchors.topMargin: 10
            x: page.margin
            width: parent.width - 20
            height: detailsText.height
            radius: guiSettings.radius
            border.width: detailsText.activeFocus ? 1 : 0
            border.color: guiSettings.buttonColor
            color: guiSettings.textInputBackgroundColor

            SkyFormattedTextEdit {
                id: detailsText
                width: parent.width
                topPadding: 10
                bottomPadding: 10
                parentPage: page
                parentFlick: flick
                placeholderText: qsTr("Please explain why you think this decision was a mistake.")
                maxLength: reportUtils.REPORT_DETAILS_SIZE
            }
        }
    }

    ReportUtils {
        id: reportUtils
        skywalker: page.skywalker // qmllint disable missing-type
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function getModerationHandle() {
        return `<a href="${moderationProfile.did}" style="color: ${guiSettings.linkColor}; text-decoration: none">@${moderationProfile.handle}</a>`
    }

    Component.onCompleted: {
        detailsText.forceActiveFocus()
    }
}
