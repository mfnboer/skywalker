import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Dialog {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property string contentAuthorDid
    required property contentlabel label
    readonly property contentgroup contentGroup: skywalker.getContentGroup(label.did, label.labelId)
    property string labelerHandle: ""
    property basicprofile labeler

    id: contentLabelInfo
    width: parent.width
    contentHeight: grid.height
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    signal appeal(contentgroup group, string labelerHandle)

    ColumnLayout {
        id: grid
        width: parent.width

        RowLayout {
            Layout.fillWidth: true

            Avatar {
                Layout.preferredWidth: 20
                userDid: contentLabelInfo.userDid
                author: labeler
            }

            SkyCleanedText {
                id: titleText
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
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: `Set by ${labelerHandle}`
            onLinkActivated: (link) => {
                skywalker.getDetailedProfile(link)
                accept()
            }
        }

        AccessibleText {
            Layout.fillWidth: true
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: label.createdAt.toLocaleString(Qt.locale(), Locale.ShortFormat)
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
        skywalker: contentLabelInfo.skywalker

        onBasicProfileOk: (profile) => { // qmllint disable signal-handler-parameters
            labeler = profile
            labelerHandle = `<a href="${profile.did}" style="color: ${guiSettings.linkColor}; text-decoration: none">@${profile.handle}</a>`
        }
    }


    function canAppeal() {
        // You can appeal to labels on your own content placed by a moderator
        return contentAuthorDid === skywalker.getUserDid() && label.did !== contentAuthorDid &&
                !label.isSystemLabel()
    }

    Component.onCompleted: {
        profileUtils.getBasicProfile(label.did)
    }
}
