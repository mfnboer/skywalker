import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property string contentAuthorDid
    required property contentlabel label
    readonly property contentgroup contentGroup: skywalker.getContentGroup(label.did, label.labelId)

    id: contentLabelInfo
    width: parent.width
    contentHeight: grid.height
    title: contentGroup.title
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent

    ColumnLayout {
        id: grid
        width: parent.width

        Text {
            id: creatorHandle
            Layout.fillWidth: true
            font.pointSize: guiSettings.scaledFont(7/8)
            elide: Text.ElideRight
            color: guiSettings.textColor
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
            maximumLineCount: 1000
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: contentGroup.formattedDescription
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: root.getSkywalker()

        onHandle: (handle, displayName, did) => {
            creatorHandle.text = `Set by <a href="${did}" style="color: ${guiSettings.linkColor}">@${handle}</a>`
        }
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        profileUtils.getHandle(label.did)
    }
}
