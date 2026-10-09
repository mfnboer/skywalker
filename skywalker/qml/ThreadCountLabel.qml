import QtQuick
import QtQuick.Controls.Material
import skywalker


SkyLabel {
    required property int postIndex
    required property int postCount
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()

    signal clicked

    labelFontHeight: guiSettings.appFontHeight * 7/8
    labelHeight: labelFontHeight + 2
    backgroundColor: guiSettings.isLightMode ? Qt.darker(guiSettings.backgroundColor, 1.09) : Qt.lighter(guiSettings.backgroundColor, 1.92)
    font.pointSize: guiSettings.scaledFont(7/8)
    text: `${userSettings.threadPrefix}${postIndex}/${postCount}`

    MouseArea {
        anchors.fill: parent
        onClicked: parent.clicked()
    }
}
