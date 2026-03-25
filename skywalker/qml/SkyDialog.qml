import QtQuick
import QtQuick.Controls.Material

Dialog {
    modal: true
    Material.background: guiSettings.backgroundColor

    enter: Transition {
        NumberAnimation { property: "scale"; from: 0; to: 1; easing.type: Easing.InOutQuad; duration: 100 }
    }

    exit: Transition {
        NumberAnimation { property: "scale"; from: 1; to: 0; easing.type: Easing.InOutQuad; duration: 100 }
    }

    onClosed: destroy()
}
