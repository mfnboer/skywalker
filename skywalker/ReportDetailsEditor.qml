import QtQuick
import QtQuick.Controls

Page {
    property alias text: detailsText.text

    signal detailsChanged(string text)

    id: page
    width: parent.width
    topPadding: 10
    bottomPadding: 10

    header: SimpleButtonHeader {
        title: qsTr("Report details")
        buttonText: qsTr("Done")
        onButtonClicked: detailsChanged(page.text)
    }

    SkyTextEdit {
        id: detailsText
        placeholderText: qsTr("Enter a reason or any other details here")
    }
}
