import QtQuick
import QtQuick.Controls
import skywalker

SvgTransparentButton {
    property var tenorSearchView: null

    signal selectedGif(tenorgif gif)

    id: addGifButton
    accessibleName: qsTr("add GIF")
    svg: SvgOutline.addGif

    onClicked: selectGif()

    function selectGif() {
        if (!tenorSearchView)
        {
            let component = guiSettings.createComponent("TenorSearch.qml")
            tenorSearchView = component.createObject(root)
            tenorSearchView.onClosed.connect(() => { root.currentStack().pop() })
            tenorSearchView.onSelected.connect((gif) => {
                    addGifButton.selectedGif(gif)
                    root.currentStack().pop()
            })
        }

        root.pushStack(tenorSearchView)
    }
}

