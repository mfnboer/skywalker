import QtQuick
import QtQuick.Controls
import skywalker

Item {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property UserSettings userSettings: skywalker.getUserSettings()
    required property basicprofile contentAuthor
    required property list<contentlabel> contentLabels
    property contentlabel filteredContentLabel
    property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(contentAuthor, contentLabels, userDid)
    property int parentWidth: parent.width
    property bool alignRight: false

    id: labelView
    width: getWidth()
    height: getHeight()
    visible: labelsToShow.length > 0

    function getWidth() {
        if (scrollViewLoader.item)
            return scrollViewLoader.item.width

        if (flowLoader.item)
            return flowLoader.item.width

        return 0
    }

    function getHeight() {
        if (scrollViewLoader.item)
            return scrollViewLoader.item.height

        if (flowLoader.item)
            return flowLoader.item.height

        return 0
    }

    Loader {
        id: scrollViewLoader
        anchors.right: parent.right
        active: !userSettings.wrapLabels

        sourceComponent: ScrollView {
            anchors.right: parent.right
            width: Math.min(labelView.parentWidth, labelRow.width)
            height: labelView.visible ? labelRow.height : 0
            contentWidth: labelRow.width
            contentHeight: height

            // To make the MouseArea below work
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff

            Row {
                id: labelRow
                topPadding: guiSettings.labelRowPadding
                spacing: 5

                ContentLabelRepeater {
                    userDid: labelView.userDid
                    contentAuthor: labelView.contentAuthor
                    filteredContentLabel: labelView.filteredContentLabel
                    labelsToShow: labelView.labelsToShow
                    parentWidth: labelView.parentWidth
                }
            }
        }
    }

    Loader {
        id: flowLoader
        active: userSettings.wrapLabels

        sourceComponent: Flow {
            width: labelView.parentWidth
            topPadding: guiSettings.labelRowPadding
            layoutDirection: labelView.alignRight ? Qt.RightToLeft : Qt.LeftToRight
            spacing: 5

            ContentLabelRepeater {
                userDid: labelView.userDid
                contentAuthor: labelView.contentAuthor
                filteredContentLabel: labelView.filteredContentLabel
                labelsToShow: labelView.labelsToShow
                parentWidth: labelView.parentWidth
            }
        }
    }
}
