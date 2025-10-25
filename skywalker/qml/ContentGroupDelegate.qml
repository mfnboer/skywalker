import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 10
    property string userDid
    property bool isLast: false
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property var model
    required property bool isSubscribed
    required property bool adultContent
    required property contentgroup contentGroup
    required property int contentPrefVisibility
    required property bool isNewLabel

    id: contentGroupView
    height: contentGroupColumn.height
    color: isNewLabel ? guiSettings.postHighLightColor : "transparent"

    ColumnLayout {
        id: contentGroupColumn
        width: parent.width

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 5
            color: "transparent"
        }

        RowLayout {
            Layout.fillWidth: true

            SkyCleanedTextLine {
                id: titleText
                Layout.leftMargin: contentGroupView.margin
                Layout.rightMargin: contentGroupView.margin
                Layout.fillWidth: true
                font.bold: true
                elide: Text.ElideRight
                color: guiSettings.textColor
                plainText: contentGroup.title
            }
            SvgButton {
                imageMargin: 0
                implicitWidth: height
                implicitHeight: titleText.height
                Layout.rightMargin: contentGroupView.margin
                svg: contentGroup.target === QEnums.LABEL_TARGET_CONTENT ? SvgOutline.chat : SvgOutline.image
                accessibleName: qsTr("target of the label")
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                visible: !contentGroup.isBadge

                onClicked: {
                    if (contentGroup.target === QEnums.LABEL_TARGET_CONTENT)
                        skywalker.showStatusMessage(qsTr("Label targets full content"), QEnums.STATUS_LEVEL_INFO)
                    else
                        skywalker.showStatusMessage(qsTr("Label targets images"), QEnums.STATUS_LEVEL_INFO)
                }
            }
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
                text: contentGroup.isBadge ? qsTr("Off") : qsTr("Show");
                visible: !contentGroup.isAdult || adultContent
                onCheckedChanged: {
                    if (checked)
                        setContentPrefVisibility(QEnums.CONTENT_PREF_VISIBILITY_SHOW)
                }

                Accessible.name: contentGroup.isBadge ? qsTr(`disable badge for ${contentGroup.text}`) : qsTr(`show ${contentGroup.text}`)
            }
            SkyRadioButton {
                Layout.fillWidth: true
                checked: contentPrefVisibility === QEnums.CONTENT_PREF_VISIBILITY_WARN
                horizontalAlignment: Qt.AlignHCenter
                text: contentGroup.isBadge ? qsTr("Show badge") : qsTr("Warn");
                visible: !contentGroup.isAdult || adultContent
                onCheckedChanged: {
                    if (checked)
                        setContentPrefVisibility(QEnums.CONTENT_PREF_VISIBILITY_WARN)
                }

                Accessible.name: contentGroup.isBadge ? qsTr(`show badge for ${contentGroup.text}`) :  qsTr(`warn for ${contentGroup.text}`)
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
                        setContentPrefVisibility(QEnums.CONTENT_PREF_VISIBILITY_HIDE)
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
                    onClicked: skywalker.showStatusMessage(qsTr("Adult content disabled"), QEnums.STATUS_LEVEL_INFO)
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: contentGroupView.isNewLabel ? guiSettings.separatorHighLightColor : guiSettings.separatorColor
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "transparent"
            visible: isLast
        }
    }


    function setContentPrefVisibility(pref) {
        if (model.contentPrefVisibility !== pref)
            model.contentPrefVisibility = pref
    }
}
