import QtQuick
import QtQuick.Controls
import skywalker

ScrollView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property string contentAuthorDid
    required property list<contentlabel> contentLabels
    property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(contentLabels, userDid)
    property int parentWidth: parent.width

    id: labelView
    width: Math.min(parentWidth, labelRow.width)
    height: visible ? labelRow.height : 0
    anchors.right: parent.right
    contentWidth: labelRow.width
    contentHeight: height
    visible: labelsToShow.length > 0

    // To make the MouseArea below work
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: ScrollBar.AlwaysOff

    Row {
        id: labelRow
        topPadding: guiSettings.labelRowPadding
        spacing: 5

        Repeater {
            model: labelsToShow

            Rectangle {
                required property contentlabel modelData
                property basicprofile labeler

                width: label.width + (labelerAvatar.active ? label.height : 0)
                height: label.height
                radius: 2
                color: modelData.did === contentAuthorDid ? guiSettings.contentUserLabelColor : guiSettings.contentLabelColor

                Loader {
                    id: labelerAvatar
                    active: labeler.avatarThumbUrl
                    asynchronous: true

                    sourceComponent: Rectangle {
                        width: height
                        height: label.height
                        color: "transparent"

                        Avatar {
                            x: 2
                            y: 2
                            width: parent.width - 4
                            author: labeler
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

                MouseArea {
                    anchors.fill: parent
                    onClicked: showInfo(modelData)
                }

                ProfileUtils {
                    id: profileUtils
                    skywalker: labelView.skywalker

                    onBasicProfileOk: (profile) => labeler = profile // qmllint disable signal-handler-parameters
                }

                Component.onCompleted: profileUtils.getBasicProfile(modelData.did)
            }
        }
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
                contentAuthorDid: contentAuthorDid,
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
