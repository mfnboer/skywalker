import QtQuick
import QtQuick.Controls

TabButton {
    id: button
    Accessible.name: qsTr(`Press to show ${text}`)
    width: implicitWidth
    display: AbstractButton.TextOnly
    font.pointSize: guiSettings.scaledFont(1)
}
