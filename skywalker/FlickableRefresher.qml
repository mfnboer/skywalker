import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Text {
    required property string topText
    required property bool inProgress
    required property int verticalOvershoot
    property var topOvershootFun: null
    property var bottomOvershootFun: null
    property bool inTopOvershoot: false
    property bool inBottomOvershoot: false

    id: refreshText
    anchors.horizontalCenter: parent.horizontalCenter
    y: (parent.headerItem ? parent.headerItem.height : 0) - verticalOvershoot - height
    z: parent.z - 1
    font.italic: true
    color: guiSettings.textColor
    visible: verticalOvershoot < 0
    text: inTopOvershoot ? qsTr("Refreshing...") : topText

    onVerticalOvershootChanged: {
        if (!enabled)
            return

        if (topOvershootFun && verticalOvershoot < -refreshText.height - 10)  {
            if (!inTopOvershoot && !inProgress) {
                topOvershootFun()
            }

            inTopOvershoot = true
        } else if (verticalOvershoot >= 0) {
            inTopOvershoot = false
        }

        if (bottomOvershootFun && verticalOvershoot > 0) {
            if (!inBottomOvershoot && !inProgress) {
                bottomOvershootFun()
            }

            inBottomOvershoot = true;
        } else {
            inBottomOvershoot = false;
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
