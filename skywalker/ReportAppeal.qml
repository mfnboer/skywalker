import QtQuick
import skywalker

SkyPage {
    required property contentlabel label
    required property contentgroup contentGroup
    required property string labelerHandle
    property var skywalker: root.getSkywalker()
    readonly property int margin: 10

    signal closed

    id: page
    width: parent.width
    topPadding: 10
    bottomPadding: 10

    header: SimpleHeader {
        text: qsTr("Appeal")
        backIsCancel: true
        onBack: { console.debug("CANCEL"); page.closed() }

        SkyButton {
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Send")
            enabled: detailsText.graphemeLength > 0 && !detailsText.maxGraphemeLengthExceeded()
            onClicked: sendAppeal()
        }
    }

    footer: Rectangle {
        id: pageFooter
        width: page.width
        height: guiSettings.footerHeight
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
        contentHeight: detailsText.y + detailsText.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        onHeightChanged: detailsText.ensureVisible(detailsText.cursorRectangle)

        SkyCleanedText {
            id: appealTitle
            width: parent.width
            leftPadding: page.margin
            rightPadding: page.margin
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.textColor
            plainText: contentGroup.title
        }

        AccessibleText {
            id: appealHeaderText
            anchors.top: appealTitle.bottom
            width: parent.width
            leftPadding: page.margin
            rightPadding: page.margin
            wrapMode: Text.Wrap
            text: qsTr(`This appeal will be sent to ${labelerHandle}`)
        }

        SkyFormattedTextEdit {
            id: detailsText
            anchors.top: appealHeaderText.bottom
            anchors.topMargin: 10
            width: parent.width
            leftPadding: page.margin
            rightPadding: page.margin
            parentPage: page
            parentFlick: flick
            placeholderText: qsTr("Please explain why you think this label was incorrectly applied.")
            maxLength: 2000
        }
    }

    ReportUtils {
        id: reportUtils
        skywalker: page.skywalker // qmllint disable missing-type

        onReportOk: {
            skywalker.showStatusMessage(qsTr("Appeal sent"), QEnums.STATUS_LEVEL_INFO)
            page.closed()
        }

        onReportFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function sendAppeal() {
        if (label.appliesToActor()) {
            reportUtils.reportAuthor(label.getActorDid(), QEnums.REPORT_REASON_TYPE_APPEAL,
                                     detailsText.text, label.did)
        }
        else {
            reportUtils.reportPostOrFeed(label.uri, label.cid, QEnums.REPORT_REASON_TYPE_APPEAL,
                                         detailsText.text, label.did)
        }
    }

    Component.onCompleted: {
        detailsText.forceActiveFocus()
    }
}
