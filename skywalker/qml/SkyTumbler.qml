import QtQuick
import QtQuick.Controls

Tumbler {
    id: hoursTumbler
    model: 24
    visibleItemCount: 3
    delegate: timeTumblerComponent
    contentItem.implicitWidth: 50
    contentItem.implicitHeight: 80

    Component {
        id: timeTumblerComponent

        AccessibleText {
            text: modelData.toString().length < 2 ? "0" + modelData : modelData
            opacity: 1.0 - Math.abs(Tumbler.displacement) / (Tumbler.tumbler.visibleItemCount / 2)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
