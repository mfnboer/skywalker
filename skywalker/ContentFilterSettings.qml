import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var model

    signal closed()

    id: page
    padding: 10

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: qsTr("Content Filtering")
        onBack: page.closed()
    }

    AccessibleSwitch {
        topPadding: 10
        bottomPadding: 20
        id: adultContentSwitch
        width: parent.width
        Material.accent: guiSettings.buttonColor
        text: qsTr("Adult content")
        checked: page.model.adultContent
        onCheckedChanged: page.model.adultContent = checked

        Accessible.role: Accessible.Button
        Accessible.name: text
        Accessible.onPressAction: toggle()
    }

    ListView {
        anchors.top: adultContentSwitch.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        clip: true
        spacing: 5
        boundsBehavior: Flickable.StopAtBounds
        model: page.model
        flickDeceleration: guiSettings.flickDeceleration

        Accessible.role: Accessible.List

        delegate: GridLayout {
            required property var model
            required property contentgroup contentGroup
            required property int contentPrefVisibility

            width: parent.width
            columns: 2

            Text {
                id: titleText
                Layout.fillWidth: true
                font.bold: true
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                text: contentGroup.title
                color: !contentGroup.isAdult || page.model.adultContent ? guiSettings.textColor : Material.color(Material.Grey)

                Accessible.role: Accessible.StaticText
                Accessible.name: `${titleText.text}\n\n${descriptionText.text}\n\npreference is: ${(getContentPrefVisibilitySpeech())}`
            }
            Row {
                id: buttonRow
                Layout.rowSpan: 2
                spacing: -1

                SkyRadioButton {
                    checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_HIDE
                    text: qsTr("Hide");
                    visible: !contentGroup.isAdult || page.model.adultContent
                    onCheckedChanged: {
                        if (checked)
                            model.contentPrefVisibility = QEnums.CONTENT_PREF_VISIBILITY_HIDE
                    }

                    Accessible.name: qsTr(`hide ${titleText.text}`)
                }
                SkyRadioButton {
                    checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_WARN
                    text: qsTr("Warn");
                    visible: !contentGroup.isAdult || page.model.adultContent
                    onCheckedChanged: {
                        if (checked)
                            model.contentPrefVisibility = QEnums.CONTENT_PREF_VISIBILITY_WARN
                    }

                    Accessible.name: qsTr(`warn for ${titleText.text}`)
                }
                SkyRadioButton {
                    checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_SHOW
                    text: qsTr("Show");
                    visible: !contentGroup.isAdult || page.model.adultContent
                    onCheckedChanged: {
                        if (checked)
                            model.contentPrefVisibility = QEnums.CONTENT_PREF_VISIBILITY_SHOW
                    }

                    Accessible.name: qsTr(`show ${titleText.text}`)
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    padding: 10
                    color: guiSettings.buttonColor
                    text: qsTr("Hide")
                    visible: contentGroup.isAdult && !page.model.adultContent

                    Accessible.ignored: true
                }
            }

            Text {
                id: descriptionText
                bottomPadding: 5
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                text: contentGroup.description
                color: !contentGroup.isAdult || page.model.adultContent ? guiSettings.textColor : Material.color(Material.Grey)

                Accessible.role: Accessible.StaticText
                Accessible.name: `${titleText.text}\n\n${descriptionText.text}\n\npreference is: ${(getContentPrefVisibilitySpeech())}`
            }

            Rectangle {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
            }

            function getContentPrefVisibilitySpeech() {
                if (contentGroup.isAdult && !page.model.adultContent)
                    return qsTr("hide, adult content is disabled")

                return accessibilityUtils.getContentPrefVisibilitySpeech(contentPrefVisibility)
            }

        }   
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        console.debug("Save filter settings");
        skywalker.saveGlobalContentFilterPreferences();
    }
}
