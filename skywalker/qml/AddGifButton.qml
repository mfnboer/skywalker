import QtQuick
import QtQuick.Controls
import skywalker

SvgTransparentButton {
    property var gifSearchView: null

    signal selectedGif(tenorgif gif)

    id: addGifButton
    accessibleName: qsTr("add GIF")
    svg: SvgOutline.addGif

    onClicked: selectGif()

    function selectGif() {
        if (!gifSearchView)
        {
            let component = guiSettings.createComponent("GifSearch.qml")
            gifSearchView = component.createObject(root)
            gifSearchView.onClosed.connect(() => { root.currentStack().pop() })
            gifSearchView.onSelected.connect((gif) => {
                    addGifButton.selectedGif(gif)
                    root.currentStack().pop()
            })
        }

        root.pushStack(gifSearchView)
        gifSearchView.init()
    }
}

