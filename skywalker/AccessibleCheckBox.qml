import QtQuick
import QtQuick.Controls

CheckBox {
    topPadding: 5
    bottomPadding: 5
    Accessible.name: text
    Accessible.onPressAction: toggle()
}
