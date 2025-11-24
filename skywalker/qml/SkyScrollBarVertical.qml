import QtQuick
import QtQuick.Controls

ScrollBar {
    width: 6
    policy: parent.contentHeight > parent.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
}
