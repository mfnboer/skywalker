import QtQuick
import QtQuick.Controls
import skywalker

ScrollView {
    required property string contentAuthorDid
    required property list<contentlabel> contentLabels
    readonly property list<contentlabel> nonSystemLabels: filterSystemLabels()
    property var skywalker: root.getSkywalker()

    id: labelView
    width: Math.min(parent.width, labelRow.width)
    height: visible ? labelRow.height : 0
    anchors.right: parent.right
    contentWidth: labelRow.width
    contentHeight: height
    visible: nonSystemLabels.length > 0

    // To make the MouseArea below work
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: ScrollBar.AlwaysOff

    Row {
        id: labelRow
        topPadding: 5
        spacing: 5

        Repeater {
            model: nonSystemLabels

            SkyLabel {
                required property contentlabel modelData

                backgroundColor: modelData.did === contentAuthorDid ? guiSettings.contentUserLabelColor : guiSettings.contentLabelColor
                font.pointSize: guiSettings.scaledFont(5/8)
                font.italic: true
                color: guiSettings.textColor
                text: getDisplayText(modelData)

                Accessible.role: Accessible.StaticText
                Accessible.name: qsTr(`content label: ${text}`)

                MouseArea {
                    anchors.fill: parent
                    onClicked: showInfo(modelData)
                }
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function getDisplayText(label) {
        const contentGroup = skywalker.getContentGroup(label.did, label.labelId)
        return contentGroup.titleWithSeverity
    }

    function filterSystemLabels() {
        let labels = []

        for (let i = 0; i < contentLabels.length; ++i) {
            if (!contentLabels[i].isSystemLabel())
                labels.push(contentLabels[i])
        }

        return labels
    }

    function showInfo(contentLabel) {
        let component = Qt.createComponent("ContentLabelInfo.qml")
        let infoPage = component.createObject(root.currentStackItem(),
                { contentAuthorDid: contentAuthorDid, label: contentLabel })
        infoPage.onAccepted.connect(() => infoPage.destroy())
        infoPage.onRejected.connect(() => infoPage.destroy())
        infoPage.open()
    }
}
