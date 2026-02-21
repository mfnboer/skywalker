import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()
    property int storageType: userSettings.getDraftStorageType()

    signal linkActivated(string link)

    id: msgDialog
    contentHeight: msgCol.height
    width: parent.width - 40
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    onOpened: msgLabel.focus = true

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: msgCol.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: SkyScrollBarVertical {}

        ColumnLayout {
            id: msgCol
            width: parent.width
            spacing: 0

            AccessibleLabel {
                id: msgLabel
                Layout.fillWidth: true

                verticalAlignment: Text.AlignVCenter
                padding: 10
                textFormat: Text.RichText
                wrapMode: Text.Wrap

                onLinkActivated: (link) => msgDialog.linkActivated(link)
            }

            SkyRoundRadioButton {
                Layout.topMargin: 10
                Layout.fillWidth: true
                text: qsTr("Save on device")
                checked: storageType === DraftPosts.STORAGE_FILE

                onCheckedChanged: {
                    if (checked)
                        storageType = DraftPosts.STORAGE_FILE
                }
            }

            SkyRoundRadioButton {
                Layout.fillWidth: true
                text: qsTr("Save on Bluesky")
                checked: storageType === DraftPosts.STORAGE_BLUESKY

                onCheckedChanged: {
                    if (checked)
                        storageType = DraftPosts.STORAGE_BLUESKY
                }
            }
        }
    }


    function show(msg) {
        msgLabel.text = msg
        open()
    }
}
