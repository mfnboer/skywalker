import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property string contentAuthorDid
    required property contentlabel label
    readonly property contentgroup contentGroup: skywalker.getContentGroup(label.did, label.labelId)
    property string labelerHandle: ""

    id: contentLabelInfo
    width: parent.width
    contentHeight: grid.height
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent

    signal appeal(contentgroup group, string labelerHandle)

    ColumnLayout {
        id: grid
        width: parent.width

        RowLayout {
            Layout.fillWidth: true

            SkyCleanedText {
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: guiSettings.textColor
                plainText: contentGroup.title
            }

            SkyButton {
                text: qsTr("Appeal")
                visible: canAppeal()
                onClicked: appeal(contentGroup, labelerHandle)
            }
        }

        AccessibleText {
            Layout.fillWidth: true
            Layout.topMargin: 10
            wrapMode: Text.Wrap
            maximumLineCount: 1000
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: contentGroup.formattedDescription
        }


        AccessibleText {
            id: creatorHandle
            Layout.fillWidth: true
            font.pointSize: guiSettings.scaledFont(7/8)
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: `Set by ${labelerHandle}`
            onLinkActivated: (link) => {
                root.getSkywalker().getDetailedProfile(link)
                accept()
            }
        }

        AccessibleText {
            Layout.fillWidth: true
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: label.createdAt.toLocaleString(Qt.locale(), Locale.LongFormat)
        }

        AccessibleText {
            Layout.fillWidth: true
            Layout.topMargin: 10
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            font.italic: true
            color: guiSettings.textColor
            text: qsTr("You may appeal this label if you feel it was placed in error.")
            visible: canAppeal()
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: root.getSkywalker()

        onHandle: (handle, displayName, did) => {
            labelerHandle = `<a href="${did}" style="color: ${guiSettings.linkColor}">@${handle}</a>`
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function canAppeal() {
        return label.did !== contentAuthorDid && !label.isSystemLabel()
    }

    Component.onCompleted: {
        profileUtils.getHandle(label.did)
    }
}
