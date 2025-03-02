import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ColumnLayout {
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    property string userDid: userSettings.getActiveUserDid()

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("Appearance")
    }

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        rowSpacing: 5

        AccessibleText {
            Layout.preferredWidth: 120
            text: qsTr("Display")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: -1

            SkyRadioButton {
                Layout.fillWidth: true
                checked: userSettings.getDisplayMode() === QEnums.DISPLAY_MODE_SYSTEM
                text: qsTr("System");
                onCheckedChanged: {
                    if (checked && userSettings.getDisplayMode() !== QEnums.DISPLAY_MODE_SYSTEM) {
                        userSettings.setDisplayMode(QEnums.DISPLAY_MODE_SYSTEM)
                        root.setDisplayMode(userSettings.getDisplayMode())
                    }
                }
            }
            SkyRadioButton {
                Layout.fillWidth: true
                checked: userSettings.getDisplayMode() === QEnums.DISPLAY_MODE_LIGHT
                text: qsTr("Light");
                onCheckedChanged: {
                    if (checked && userSettings.getDisplayMode() !== QEnums.DISPLAY_MODE_LIGHT) {
                        userSettings.setDisplayMode(QEnums.DISPLAY_MODE_LIGHT)
                        root.setDisplayMode(userSettings.getDisplayMode())
                    }
                }
            }
            SkyRadioButton {
                Layout.fillWidth: true
                checked: userSettings.getDisplayMode() === QEnums.DISPLAY_MODE_DARK
                text: qsTr("Dark");
                onCheckedChanged: {
                    if (checked && userSettings.getDisplayMode() !== QEnums.DISPLAY_MODE_DARK) {
                        userSettings.setDisplayMode(QEnums.DISPLAY_MODE_DARK)
                        root.setDisplayMode(userSettings.getDisplayMode())
                    }
                }
            }
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Background color")
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            border.width: 1
            border.color: guiSettings.buttonColor
            color: guiSettings.backgroundColor

            MouseArea {
                anchors.fill: parent
                onClicked: selectBackgroundColor()
            }

            SvgPlainButton {
                y: -2
                anchors.right: parent.right
                width: height
                height: 34
                Material.background: guiSettings.backgroundColor
                svg: SvgOutline.close
                accessibleName: qsTr("reset background color to default")
                onClicked: userSettings.resetBackgroundColor()
            }
        }

        Rectangle {
            Layout.preferredWidth: 120
            Layout.preferredHeight: accentColorLabel.height
            color: "transparent"

            AccessibleText {
                id: accentColorLabel
                width: parent.width - accentColorInfoButton.width
                wrapMode: Text.Wrap
                text: qsTr("Accent color")
            }
            SvgButton {
                id: accentColorInfoButton
                anchors.right: parent.right
                width: 34
                height: width
                imageMargin: 4
                svg: SvgOutline.info
                accessibleName: qsTr("info")
                onClicked: guiSettings.notice(root, qsTr("Accent color is used for buttons, default avatar and counter badges"))
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            border.width: 1
            border.color: guiSettings.accentColor
            color: guiSettings.accentColor

            MouseArea {
                anchors.fill: parent
                onClicked: selectAccentColor()
            }

            SvgPlainButton {
                y: -2
                anchors.right: parent.right
                width: height
                height: 34
                Material.background: guiSettings.backgroundColor
                svg: SvgOutline.close
                accessibleName: qsTr("reset accent color to default")
                onClicked: {
                    userSettings.resetAccentColor()
                    root.Material.accent = userSettings.accentColor
                }
            }
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Link color")
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            border.width: 1
            border.color: guiSettings.buttonColor
            color: guiSettings.linkColor

            MouseArea {
                anchors.fill: parent
                onClicked: selectLinkColor()
            }

            SvgPlainButton {
                y: -2
                anchors.right: parent.right
                width: height
                height: 34
                Material.background: guiSettings.backgroundColor
                svg: SvgOutline.close
                accessibleName: qsTr("reset link color to default")
                onClicked: userSettings.resetLinkColor()
            }
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Post thread visualisation")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: -1

            SkyRadioButton {
                Layout.fillWidth: true
                checked: userSettings.threadStyle === QEnums.THREAD_STYLE_BAR
                text: qsTr("Bar");
                onCheckedChanged: {
                    if (checked)
                        userSettings.threadStyle = QEnums.THREAD_STYLE_BAR
                }
            }
            SkyRadioButton {
                Layout.fillWidth: true
                checked: userSettings.threadStyle === QEnums.THREAD_STYLE_LINE
                text: qsTr("Line");
                onCheckedChanged: {
                    if (checked)
                        userSettings.threadStyle = QEnums.THREAD_STYLE_LINE
                }
            }
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Post thread color")
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            border.width: 1
            border.color: guiSettings.buttonColor

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: guiSettings.threadStartColor(userSettings.threadColor) }
                GradientStop { position: 0.7; color: guiSettings.threadMidColor(userSettings.threadColor) }
                GradientStop { position: 1.0; color: guiSettings.threadEndColor(userSettings.threadColor) }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: selectThreadColor()
            }

            SvgPlainButton {
                y: -2
                anchors.right: parent.right
                width: height
                height: 34
                Material.background: guiSettings.backgroundColor
                svg: SvgOutline.close
                accessibleName: qsTr("reset thread color to default")
                onClicked: userSettings.resetThreadColor()
            }
        }
    }

    AccessibleCheckBox {
        text: qsTr("Floating navigation buttons")
        checked: userSettings.floatingNavigationButtons
        onCheckedChanged: userSettings.floatingNavigationButtons = checked
    }

    AccessibleCheckBox {
        text: qsTr("Giant emoji")
        checked: userSettings.giantEmojis
        onCheckedChanged: userSettings.giantEmojis = checked
    }

    AccessibleCheckBox {
        text: qsTr("GIF auto play")
        checked: userSettings.getGifAutoPlay()
        onCheckedChanged: userSettings.setGifAutoPlay(checked)
    }

    AccessibleCheckBox {
        text: qsTr("Video auto play")
        checked: userSettings.videoAutoPlay
        onCheckedChanged: userSettings.videoAutoPlay = checked
    }

    AccessibleCheckBox {
        text: qsTr("Video auto load")
        checked: userSettings.videoAutoLoad
        onCheckedChanged: userSettings.videoAutoLoad = checked
        enabled: !userSettings.videoAutoPlay
    }
    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("With auto loading, the video is automatically loaded (more data usage) so it starts faster when you press play. Otherwise it will load when you press play.")
    }

    AccessibleCheckBox {
        text: qsTr("Video loop play")
        checked: userSettings.videoLoopPlay
        onCheckedChanged: userSettings.videoLoopPlay = checked
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: -1

        AccessibleText {
            Layout.preferredWidth: 120
            text: qsTr("Video quality")
        }
        SkyRadioButton {
            Layout.fillWidth: true
            checked: userSettings.videoQuality === QEnums.VIDEO_QUALITY_HD
            text: qsTr("HD");
            onCheckedChanged: {
                if (checked)
                    userSettings.videoQuality = QEnums.VIDEO_QUALITY_HD
            }
        }
        SkyRadioButton {
            Layout.fillWidth: true
            checked: userSettings.videoQuality === QEnums.VIDEO_QUALITY_HD_WIFI
            text: qsTr("HD WiFi");
            onCheckedChanged: {
                if (checked)
                    userSettings.videoQuality = QEnums.VIDEO_QUALITY_HD_WIFI
            }
        }
        SkyRadioButton {
            Layout.fillWidth: true
            checked: userSettings.videoQuality === QEnums.VIDEO_QUALITY_SD
            text: qsTr("SD");
            onCheckedChanged: {
                if (checked)
                    userSettings.videoQuality = QEnums.VIDEO_QUALITY_SD
            }
        }
    }
    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Higher quality requires more network bandwidth (data).<br>High Definition (HD) is 1280x720 pixels.<br>Standard Definition (SD) is 640x360 pixels.<br>HD WiFi uses HD when you are connected to WiFi.")
    }

    Utils {
        id: utils
    }

    function selectThreadColor() {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = userSettings.threadColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            userSettings.threadColor = cs.selectedColor
            cs.destroy()
        })
        cs.open()
    }

    function selectBackgroundColor() {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = userSettings.backgroundColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            if (checkColor(cs.selectedColor, guiSettings.forbiddenBackgroundColors()))
                userSettings.backgroundColor = cs.selectedColor
            else
                skywalker.showStatusMessage(qsTr("This color would make parts of the app invisible. Pick another color"), QEnums.STATUS_LEVEL_ERROR)
            cs.destroy()
        })
        cs.open()
    }

    function selectAccentColor() {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = userSettings.accentColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            if (checkColor(cs.selectedColor, guiSettings.forbiddenAccentColors())) {
                userSettings.accentColor = cs.selectedColor
                root.Material.accent = cs.selectedColor
            }
            else {
                skywalker.showStatusMessage(qsTr("This color would make parts of the app invisible. Pick another color"), QEnums.STATUS_LEVEL_ERROR)
            }
            cs.destroy()
        })
        cs.open()
    }

    function selectLinkColor() {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = userSettings.accentColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            if (checkColor(cs.selectedColor, guiSettings.forbiddenLinkColors()))
                userSettings.linkColor = cs.selectedColor
            else
                skywalker.showStatusMessage(qsTr("This color would make some links invisible. Pick another color"), QEnums.STATUS_LEVEL_ERROR)
            cs.destroy()
        })
        cs.open()
    }

    function checkColor(color, forbiddenColors) {
        for (let i = 0; i < forbiddenColors.length; ++i)
        {
            if (utils.similarColors(color, forbiddenColors[i]))
                return false
        }

        return true
    }
}
