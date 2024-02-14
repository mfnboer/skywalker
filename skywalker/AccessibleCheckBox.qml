import QtQuick
import QtQuick.Controls

CheckBox {
    Accessible.name: text
    Accessible.onPressAction: toggle()
}
