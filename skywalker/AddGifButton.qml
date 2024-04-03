import QtQuick
import QtQuick.Controls
import skywalker

SvgImage {
    property var tenorSearchView: null
    property bool enabled: true

    signal selectedGif(tenorgif gif)

    id: addGifButton
    color: addGifButton.enabled ? guiSettings.buttonColor : guiSettings.disabledColor
    opacity: 1
    svg: svgOutline.addGif

    Rectangle {
        y: -parent.height
        width: parent.width
        height: parent.height
        color: "transparent"

        Accessible.role: Accessible.Button
        Accessible.name: qsTr("add GIF")
        Accessible.onPressAction: if (addGifButton.enabled) parent.selectGif()
    }

    function selectGif() {
        if (!tenorSearchView)
        {
            let component = Qt.createComponent("TenorSearch.qml")
            tenorSearchView = component.createObject(root)
            tenorSearchView.onClosed.connect(() => { root.currentStack().pop() })
            tenorSearchView.onSelected.connect((gif) => {
                    addGifButton.selectedGif(gif)
                    root.currentStack().pop()
            })
        }

        root.pushStack(tenorSearchView)
    }

    MouseArea {
        y: -parent.height
        width: parent.width
        height: parent.height
        enabled: addGifButton.enabled
        onClicked: addGifButton.selectGif()
    }

    GuiSettings {
        id: guiSettings
    }
}

