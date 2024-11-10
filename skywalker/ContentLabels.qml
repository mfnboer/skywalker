import QtQuick
import QtQuick.Controls
import skywalker

ScrollView {
    property var skywalker: root.getSkywalker()
    required property string contentAuthorDid
    required property list<contentlabel> contentLabels
    property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(contentLabels)
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


    function getDisplayText(label) {
        const contentGroup = skywalker.getContentGroup(label.did, label.labelId)
        return contentGroup.titleWithSeverity
    }

    function appeal(contentLabel, contentGroup, labelerHandle) {
        let component = Qt.createComponent("ReportAppeal.qml")
        let page = component.createObject(root.currentStackItem(), {
            label: contentLabel,
            contentGroup: contentGroup,
            labelerHandle: labelerHandle
        })
        page.onClosed.connect(() => root.popStack())
        root.pushStack(page)
    }

    function showInfo(contentLabel) {
        let component = Qt.createComponent("ContentLabelInfo.qml")
        let infoPage = component.createObject(root.currentStackItem(),
                { contentAuthorDid: contentAuthorDid, label: contentLabel })
        infoPage.onAccepted.connect(() => infoPage.destroy())
        infoPage.onRejected.connect(() => infoPage.destroy())
        infoPage.onAppeal.connect((contentGroup, labelerHandle) => {
            appeal(contentLabel, contentGroup, labelerHandle)
            infoPage.destroy()
        })
        infoPage.open()
    }
}
