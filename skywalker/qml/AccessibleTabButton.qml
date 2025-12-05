import QtQuick
import QtQuick.Controls

TabButton {
    property real origPointSize
    readonly property real fontScaleFactor: guiSettings.fontScaleFactor

    id: button
    Accessible.name: qsTr(`Press to show ${text}`)
    width: implicitWidth
    display: AbstractButton.TextOnly

    onFontScaleFactorChanged: button.font.pointSize = origPointSize * fontScaleFactor

    Component.onCompleted: {
        origPointSize = button.font.pointSize
        button.font.pointSize = origPointSize * fontScaleFactor
    }
}
