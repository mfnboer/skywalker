import QtQuick
import skywalker

Repeater {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property basicprofile contentAuthor
    property contentlabel filteredContentLabel
    property list<contentlabel> labelsToShow
    property int parentWidth: parent.width

    model: labelsToShow

    Rectangle {
        required property contentlabel modelData
        property basicprofile labeler

        id: labelRect
        width: label.width + (labelerAvatar.active ? label.height : 0)
        height: label.height
        radius: 2
        color: getLabelColor()

        function getLabelColor() {
            if (modelData.did === filteredContentLabel.did && modelData.labelId === filteredContentLabel.labelId)
                return guiSettings.hideReasonLabelColor

            return modelData.did === contentAuthor.did ? guiSettings.contentUserLabelColor : guiSettings.contentLabelColor
        }

        Loader {
            id: labelerAvatar
            active: labelRect.labeler.avatarThumbUrl
            asynchronous: true

            sourceComponent: Rectangle {
                width: height
                height: label.height
                color: "transparent"

                Avatar {
                    x: 2
                    y: 2
                    width: parent.width - 4
                    author: labelRect.labeler
                    showModeratorIcon: false
                }
            }
        }

        SkyLabel {
            id: label
            anchors.left: labelerAvatar.right
            backgroundColor: "transparent"
            font.pointSize: guiSettings.scaledFont(6/8)
            font.italic: true
            color: guiSettings.textColor
            text: getDisplayText(modelData)

            Accessible.role: Accessible.StaticText
            Accessible.name: qsTr(`content label: ${text}`)
        }

        SkyMouseArea {
            anchors.fill: parent
            onClicked: showInfo(modelData)
        }

        ProfileUtils {
            id: profileUtils
            skywalker: labelView.skywalker

            onBasicProfileOk: (profile) => labelRect.labeler = profile
        }

        Component.onCompleted: profileUtils.getBasicProfile(modelData.did)
    }

    function getDisplayText(label) {
        const contentGroup = skywalker.getContentGroup(label.did, label.labelId)

        if (contentGroup)
            return contentGroup.titleWithSeverity

        return "unknown"
    }

    function appeal(contentLabel, contentGroup, labelerHandle) {
        let component = guiSettings.createComponent("ReportAppeal.qml")
        let page = component.createObject(root.currentStackItem(), {
            userDid: labelView.userDid,
            label: contentLabel,
            contentGroup: contentGroup,
            labelerHandle: labelerHandle
        })
        page.onClosed.connect(() => root.popStack())
        root.pushStack(page)
    }

    function showInfo(contentLabel) {
        let component = guiSettings.createComponent("ContentLabelInfo.qml")
        let infoPage = component.createObject(root.currentStackItem(), {
                userDid: labelView.userDid,
                contentAuthorDid: contentAuthor.did,
                label: contentLabel
        })
        infoPage.onAccepted.connect(() => infoPage.destroy())
        infoPage.onRejected.connect(() => infoPage.destroy())
        infoPage.onAppeal.connect((contentGroup, labelerHandle) => {
            appeal(contentLabel, contentGroup, labelerHandle)
            infoPage.destroy()
        })
        infoPage.open()
    }
}
