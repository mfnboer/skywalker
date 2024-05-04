import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property string contentAuthorDid
    required property contentlabel label
    readonly property string contentWarning: skywalker.getContentWarning([label])

    id: contentLabelInfo
    width: parent.width
    contentHeight: grid.height
    title: qsTr("Content label")
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent

    GridLayout {
        id: grid
        width: parent.width
        columns: 2
        columnSpacing: 10

        Text {
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Label:")
        }
        Text {
            Layout.fillWidth: true
            color: guiSettings.textColor
            elide: Text.ElideRight
            text: label.labelId
        }

        Text {
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Creator:")
        }
        SkyCleanedText {
            id: creatorText
            Layout.fillWidth: true
            color: guiSettings.textColor
            elide: Text.ElideRight
            plainText: ""
        }

        Item {}
        Text {
            id: creatorHandle
            Layout.fillWidth: true
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            elide: Text.ElideRight
            onLinkActivated: (link) => {
                root.getSkywalker().getDetailedProfile(link)
                accept()
            }
        }

        Text {
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Date:")
        }
        Text {
            Layout.fillWidth: true
            color: guiSettings.textColor
            elide: Text.ElideRight
            text: label.createdAt.toLocaleString(Qt.locale(), Locale.LongFormat)
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: root.getSkywalker()

        onHandle: (handle, displayName, did) => {
            creatorText.plainText = displayName.length > 0 ? displayName : handle
            creatorHandle.text = `<a href="${did}" style="color: ${guiSettings.linkColor}">@${handle}</a>`
        }
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        profileUtils.getHandle(label.did)
    }
}
