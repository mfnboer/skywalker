import QtQuick
import QtQuick.Controls

ComboBox {
    property string host
    property bool valid: true

    id: hostField
    model: ["bsky.social", "eurosky.social"]
    editable: true
    editText: host
    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
    activeFocusOnTab: false

    Accessible.role: Accessible.ComboBox
    Accessible.name: qsTr(`Hosting provider ${editText}`)
    Accessible.description: qsTr("Choose hosting provider to sign into")

    Rectangle {
        z: parent.z - 1
        width: hostField.width
        height: hostField.height
        radius: 5
        color: valid ? guiSettings.textInputBackgroundColor : guiSettings.textInputInvalidColor
    }

    function init() {
        if (!host)
            return

        const index = find(host)

        if (index >= 0) {
            currentIndex = index
            return
        }

        model.push(host)
        currentIndex = model.length - 1
    }
}
