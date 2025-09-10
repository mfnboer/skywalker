import QtQuick
import QtQuick.Controls

TabButton {
    Accessible.name: qsTr(`Press to show ${text}`)
    width: implicitWidth
    display: AbstractButton.TextOnly
}
