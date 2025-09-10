import QtQuick
import QtQuick.Controls

Switch {
    Accessible.role: Accessible.Button
    Accessible.name: text
    Accessible.onPressAction: toggle()
}
