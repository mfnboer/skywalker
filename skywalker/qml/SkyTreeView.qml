import QtQuick
import QtQuick.Controls

TreeView {
    readonly property int count: rows
    readonly property string error: (model && typeof model.error != 'undefined') ? model.error : ""
}
