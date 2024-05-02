import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ColumnLayout {
    property int margin: 10
    required property var model
    required property bool isSubscribed
    required property bool adultContent
    required property contentgroup contentGroup
    required property int contentPrefVisibility

    id: contentGroupView

    SkyCleanedText {
        Layout.leftMargin: contentGroupView.margin
        Layout.rightMargin: contentGroupView.margin
        Layout.fillWidth: true
        font.bold: true
        elide: Text.ElideRight
        color: guiSettings.textColor
        plainText: contentGroup.title
    }

    AccessibleText {
        Layout.leftMargin: contentGroupView.margin
        Layout.rightMargin: contentGroupView.margin
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        maximumLineCount: 1000
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: guiSettings.textColor
        text: contentGroup.formattedDescription
    }

    RowLayout {
        id: buttonRow
        Layout.fillWidth: true
        spacing: -1
        visible: isSubscribed

        SkyRadioButton {
            Layout.leftMargin: contentGroupView.margin
            Layout.fillWidth: true
            checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_SHOW
            horizontalAlignment: Qt.AlignHCenter
            text: qsTr("Show");
            visible: !contentGroup.isAdult || adultContent
            onCheckedChanged: {
                if (checked)
                    model.contentPrefVisibility = QEnums.CONTENT_PREF_VISIBILITY_SHOW
            }

            Accessible.name: qsTr(`show ${contentGroup.text}`)
        }
        SkyRadioButton {
            Layout.fillWidth: true
            checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_WARN
            horizontalAlignment: Qt.AlignHCenter
            text: qsTr("Warn");
            visible: !contentGroup.isAdult || adultContent
            onCheckedChanged: {
                if (checked)
                    model.contentPrefVisibility = QEnums.CONTENT_PREF_VISIBILITY_WARN
            }

            Accessible.name: qsTr(`warn for ${contentGroup.text}`)
        }
        SkyRadioButton {
            Layout.rightMargin: contentGroupView.margin
            Layout.fillWidth: true
            checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_HIDE
            horizontalAlignment: Qt.AlignHCenter
            text: qsTr("Hide");
            visible: !contentGroup.isAdult || adultContent
            onCheckedChanged: {
                if (checked)
                    model.contentPrefVisibility = QEnums.CONTENT_PREF_VISIBILITY_HIDE
            }

            Accessible.name: qsTr(`hide ${contentGroup.text}`)
        }

        Text {
            Layout.fillWidth: true
            padding: 10
            color: guiSettings.buttonColor
            horizontalAlignment: Qt.AlignHCenter
            text: qsTr("Hide")
            visible: contentGroup.isAdult && !adultContent

            Accessible.ignored: true

            MouseArea {
                anchors.fill: parent
                onClicked: root.getSkywalker().showStatusMessage(qsTr("Adult content disabled"), QEnums.STATUS_LEVEL_INFO)
            }
        }
    }

    SeparatorLine {
        Layout.fillWidth: true
    }

    GuiSettings {
        id: guiSettings
    }
}
