import QtQuick
import QtQuick.Controls
import skywalker

ScrollView {
    required property list<language> languageLabels

    id: labelView
    width: Math.min(parent.width, labelRow.width)
    height: visible ? labelRow.height + 2 : 0
    anchors.right: parent.right
    contentWidth: labelRow.width
    contentHeight: height
    visible: languageLabels.length > 0

    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: ScrollBar.AlwaysOff

    Row {
        id: labelRow
        topPadding: 5
        spacing: 5

        Repeater {
            model: languageLabels

            SkyLabel {
                required property language modelData

                backgroundColor: guiSettings.contentLabelColor
                font.pointSize: guiSettings.scaledFont(5/8)
                font.italic: true
                color: guiSettings.textColor
                text: modelData.shortCode

                Accessible.role: Accessible.StaticText
                Accessible.name: qsTr(`language indication: ${text}`)

                MouseArea {
                    anchors.fill: parent
                    onClicked: showInfo(modelData)
                }
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function showInfo(language) {
        root.getSkywalker().showStatusMessage(language.nativeName, QEnums.STATUS_LEVEL_INFO)
    }
}
