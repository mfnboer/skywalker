import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    property date expiresAt
    property date nullDate

    height: durationGrid.height

    ButtonGroup {
        id: durationGroup
    }

    GridLayout {
        id: durationGrid
        width: parent.width
        rowSpacing: 0
        columns: 2

        SkyRoundRadioButton {
            id: foreverButton
            text: qsTr("Forever")
            ButtonGroup.group: durationGroup
            onCheckedChanged: {
                if (checked)
                    expiresAt = nullDate
            }
        }
        SkyRoundRadioButton {
            text: qsTr("24 hours")
            ButtonGroup.group: durationGroup
            onCheckedChanged: {
                if (checked)
                    setExpiresAtDays(1)
            }
        }
        SkyRoundRadioButton {
            text: qsTr("7 days")
            ButtonGroup.group: durationGroup
            onCheckedChanged: {
                if (checked)
                    setExpiresAtDays(7)
            }
        }
        SkyRoundRadioButton {
            text: qsTr("30 days")
            ButtonGroup.group: durationGroup
            onCheckedChanged: {
                if (checked)
                    setExpiresAtDays(30)
            }
        }

        RowLayout {
            Layout.columnSpan: 2

            SkyRoundRadioButton {
                id: untilButton
                text: qsTr("Until:")
                ButtonGroup.group: durationGroup
            }
            SkyTextInput {
                id: untilIntput
                Layout.fillWidth: true
                svgIcon: SvgOutline.date
                placeholderText: qsTr("Date, time")
                text: enabled ? expiresAt.toLocaleString(Qt.locale(), Locale.ShortFormat) : ""
                enabled: untilButton.checked

                MouseArea {
                    anchors.fill: parent
                    onClicked: selectExpiresDate()
                }
            }
        }
    }

    function setExpiresAtDays(days) {
        expiresAt = new Date()
        expiresAt.setDate(expiresAt.getDate() + days)
    }

    function selectExpiresDate() {
        if (isNaN(expiresAt.getTime()))
            setExpiresAtDays(1)

        let component = guiSettings.createComponent("DatePicker.qml")
        let datePicker = component.createObject(parent, { selectedDate: expiresAt, enableTime: true })
        datePicker.onRejected.connect(() => datePicker.destroy())

        datePicker.onAccepted.connect(() => {
            expiresAt = datePicker.selectedDate
            datePicker.destroy()
        })

        datePicker.open()
    }

    Component.onCompleted: {
        if (!isNaN(expiresAt.getTime()))
            untilButton.checked = true
        else
            foreverButton.checked = true
    }
}
