import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property date selectedDate
    property bool enableTime: false
    readonly property int firstYear: 2020
    property bool initialized: false

    id: datePicker
    x: (parent.width - width) / 2
    contentWidth: Math.max(yearComboBox.x + yearComboBox.width, monthGrid.width)
    contentHeight: timeRow.visible ? timeRow.y + timeRow.height : monthColumn.y + monthColumn.height
    topMargin: guiSettings.headerHeight + 10
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    Material.background: guiSettings.backgroundColor

    ComboBox {
        id: monthComboBox
        implicitContentWidthPolicy: ComboBox.WidestText
        width: implicitContentWidth + implicitIndicatorWidth
        anchors.left: parent.left
        anchors.top: parent.top
        background.implicitHeight: implicitIndicatorHeight + 10
        onCurrentIndexChanged: setSelectedDate()

        function getMonth() {
            return Math.max(currentIndex, 0)
        }
    }

    ComboBox {
        id: yearComboBox
        implicitContentWidthPolicy: ComboBox.WidestText
        width: implicitContentWidth + implicitIndicatorWidth
        anchors.left: monthComboBox.right
        anchors.leftMargin: 10
        anchors.top: parent.top
        background.implicitHeight: implicitIndicatorHeight + 10
        onCurrentIndexChanged: setSelectedDate()

        function getYear() {
            return Math.max(currentIndex, 0) + firstYear
        }
    }

    ColumnLayout {
        id: monthColumn
        anchors.top: monthComboBox.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        DayOfWeekRow {
            id: dayOfWeekRow
            locale: monthGrid.locale
            Layout.fillWidth: true

            delegate: Text {
                text: shortName
                color: guiSettings.textColor
                font: dayOfWeekRow.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                required property string shortName
            }
        }

        MonthGrid {
            property int day: 1

            id: monthGrid
            Layout.fillWidth: true
            month: monthComboBox.getMonth()
            year: yearComboBox.getYear()
            locale: Qt.locale()

            delegate: Text {
                required property var model
                readonly property bool selected: model.day === monthGrid.day

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                opacity: model.month === monthGrid.month ? 1 : 0
                color: selected ? guiSettings.buttonTextColor : guiSettings.buttonColor
                text: model.day

                Rectangle {
                    anchors.fill: parent
                    z: parent.z - 1
                    color: selected ? guiSettings.buttonColor : "transparent"
                }
            }

            onMonthChanged: checkLastDay()
            onYearChanged: checkLastDay()

            onClicked: (date) => {
                day = date.getDate()
                setSelectedDate()
            }

            function checkLastDay() {
                const numDays = new Date(year, (month + 1) % 12, 0).getDate()

                if (day > numDays)
                    day = numDays
            }
        }
    }

    RowLayout {
        id: timeRow
        anchors.top: monthColumn.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        visible: enableTime

        AccessibleText {
            text: "Time:"
        }
        SkyTumbler {
            id: hoursTumbler
            model: 24
            onCurrentIndexChanged: setSelectedDate()
        }
        Text {
            color: guiSettings.textColor
            text: ":"
        }
        SkyTumbler {
            id: minutesTumbler
            model: 60
            onCurrentIndexChanged: setSelectedDate()
        }
    }

    function setSelectedDate() {
        if (!initialized)
            return

        selectedDate.setFullYear(yearComboBox.getYear(), monthComboBox.getMonth(), monthGrid.day)

        if (enableTime)
            selectedDate.setHours(hoursTumbler.currentIndex, minutesTumbler.currentIndex)
    }

    Component.onCompleted: {
        let months = []

        for (let i = 0; i < 12; ++i)
             months.push(Qt.locale().monthName(i, Locale.LongFormat))

        monthComboBox.model = months

        let years = []
        const today = new Date()

        for (let j = firstYear; j <= today.getFullYear() + 1; ++j)
            years.push(j.toString())

        yearComboBox.model = years

        yearComboBox.currentIndex = selectedDate.getFullYear() - firstYear
        monthComboBox.currentIndex = selectedDate.getMonth()
        monthGrid.day = selectedDate.getDate()

        if (enableTime) {
            hoursTumbler.currentIndex = selectedDate.getHours()
            minutesTumbler.currentIndex = selectedDate.getMinutes()
        }

        initialized = true
    }
}
