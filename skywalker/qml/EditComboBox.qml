import QtQuick
import QtQuick.Controls

ComboBox {
    property int radius: guiSettings.radius
    required property int editableIndex
    property string initialEditValue
    property string borderColor: guiSettings.buttonColor
    property string textColor: guiSettings.textColor
    property bool indexChangeEnabled: true

    signal inputTextChanged(var textInput)
    signal editingFinished(string text)

    id: comboBox
    height: textInput.height
    editable: currentIndex === editableIndex

    onCurrentIndexChanged: {
        if (currentIndex === editableIndex) {
            textInput.text = initialEditValue

            if (indexChangeEnabled) {
                textInput.forceActiveFocus()
                Qt.inputMethod.show()
            }
        }
        else {
            textInput.text = textAt(currentIndex)
        }
    }

    contentItem: WordsInput {
        id: textInput
        width: parent.width
        leftPadding: 10
        verticalAlignment: Text.AlignVCenter
        color: comboBox.textColor
        enabled: comboBox.editable
        font.pointSize: guiSettings.scaledFont(9/8)
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
        text: comboBox.displayText
        clip: true

        onDisplayTextChanged: {
            if (comboBox.currentIndex === comboBox.editableIndex)
                comboBox.inputTextChanged(textInput)
        }

        onEditingFinished: {
            if (comboBox.currentIndex === comboBox.editableIndex)
                comboBox.editingFinished(text)
        }

        onAccepted: {
            if (comboBox.currentIndex === comboBox.editableIndex) {
                comboBox.editingFinished(text)

                // When the user pressed the ENTER KEY on Android, then the currentIndex
                // gets set to -1 after onAccepted is done. I don't now why. With
                // this timer we set it back to the editable entry.
                acceptedTimer.start()
            }
        }
    }

    // For some reason the background item gets positioned too low??
    // background: Item {}
    background: Rectangle {
        implicitWidth: 140
        implicitHeight: 34
        radius: comboBox.radius
        border.color: comboBox.borderColor
        border.width: comboBox.activeFocus ? 1 : 0
        color: guiSettings.textInputBackgroundColor
    }

    Timer {
        id: acceptedTimer
        interval: 100
        onTriggered: currentIndex = editableIndex
    }


    function setSelection(value) {
        indexChangeEnabled = false
        comboBox.currentIndex = comboBox.indexOfValue(value)
        indexChangeEnabled = true
    }
}
