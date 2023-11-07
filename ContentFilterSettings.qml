import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var model

    signal closed()

    id: page
    padding: 10

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout
        {
            id: headerRow

            SvgButton {
                id: backButton
                iconColor: "white"
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: page.closed()
            }
            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: qsTr("Content Filtering")
            }
        }
    }

    Switch {
        topPadding: 10
        bottomPadding: 20
        id: adultContentSwitch
        width: parent.width
        Material.accent: guiSettings.buttonColor
        text: qsTr("Adult content")
        checked: page.model.adultContent
        onCheckedChanged: page.model.adultContent = checked
    }

    ListView {
        anchors.top: adultContentSwitch.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        clip: true
        spacing: 5
        boundsBehavior: Flickable.StopAtBounds
        model: page.model

        delegate: GridLayout {
            required property var model
            required property contentgroup contentGroup
            required property int contentPrefVisibility

            width: parent.width
            columns: 2

            Text {
                Layout.fillWidth: true
                font.bold: true
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                text: contentGroup.title
                color: !contentGroup.isAdult || page.model.adultContent ? guiSettings.textColor : Material.color(Material.Grey)
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
                }
                SkyRadioButton {
                    checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_WARN
                    text: qsTr("Warn");
                    visible: !contentGroup.isAdult || page.model.adultContent
                    onCheckedChanged: {
                        if (checked)
                            model.contentPrefVisibility = QEnums.CONTENT_PREF_VISIBILITY_WARN
                    }
                }
                SkyRadioButton {
                    checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_SHOW
                    text: qsTr("Show");
                    visible: !contentGroup.isAdult || page.model.adultContent
                    onCheckedChanged: {
                        if (checked)
                            model.contentPrefVisibility = QEnums.CONTENT_PREF_VISIBILITY_SHOW
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    padding: 10
                    color: guiSettings.buttonColor
                    text: qsTr("Hide")
                    visible: contentGroup.isAdult && !page.model.adultContent
                }
            }

            Text {
                bottomPadding: 5
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                text: contentGroup.subTitle
                color: !contentGroup.isAdult || page.model.adultContent ? guiSettings.textColor : Material.color(Material.Grey)
            }

            Rectangle {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
            }
        }

        GuiSettings {
            id: guiSettings
        }
    }

    Component.onDestruction: {
        // TODO: leave page only after saving is done, show busy indicator
        console.debug("Save filter settings");
        skywalker.saveContentFilterPreferences();
    }
}
