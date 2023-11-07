import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property string imgSource
    property alias text: altText.text

    signal altTextChanged(string text)

    id: page
    width: parent.width
    topPadding: 10
    bottomPadding: 10

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        Text {
            id: headerTexts
            anchors.leftMargin: 10
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: "white"
            text: qsTr("ALT text")
        }

        SkyButton {
            id: postButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Done")
            onClicked: altTextChanged(page.text)
        }
    }

    TextEdit {
        id: altText
        width: page.width
        leftPadding: 10
        rightPadding: 10
        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.Wrap
        font.pointSize: guiSettings.scaledFont(9/8)
        color: guiSettings.textColor
        clip: true
        focus: true

        Text {
            anchors.fill: parent
            leftPadding: altText.leftPadding
            rightPadding: altText.rightPadding
            font.pointSize: guiSettings.scaledFont(7/8)
            color: Material.color(Material.Grey)
            elide: Text.ElideRight
            text: qsTr("Description to help users with visual impairments")
            visible: altText.length + altText.preeditText.length === 0
        }
    }

    Image {
        anchors.leftMargin: 10
        anchors.left: parent.left
        anchors.topMargin: 10
        anchors.top: altText.bottom
        width: 240
        height: 180
        fillMode: Image.PreserveAspectCrop
        autoTransform: true
        source: page.imgSource
    }

    GuiSettings {
        id: guiSettings
    }
}
