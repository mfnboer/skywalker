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

    header: SimpleButtonHeader {
        title: qsTr("ALT text")
        buttonText: qsTr("Done")
        onButtonClicked: altTextChanged(page.text)
    }

    SkyTextEdit {
        id: altText
        placeholderText: qsTr("Description to help users with visual impairments")
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
}
