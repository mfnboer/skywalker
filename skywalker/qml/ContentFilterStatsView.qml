import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property string userDid
    required property ContentFilterStatsModel model

    id: page

    signal closed

    header: SimpleHeader {
        text: qsTr("Filter statistics")
        onBack: page.closed()
    }

    footer: DeadFooterMargin {
    }

    TreeView {
        id: treeView
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        model: page.model

        delegate: TreeViewDelegate {
            required property int column
            required property int valueType // QEnums::ValueType
            required property var value

            id: control
            implicitWidth: column == 1 ? 70 : treeView.width - 70

            contentItem: Loader {
                active: true
                sourceComponent: control.valueType === QEnums.VALUE_TYPE_BASIC_PROFILE ? authorComp : labelComp
            }

            Component {
                id: labelComp

                Label {
                    rightPadding: 10
                    clip: false
                    text: control.value
                    color: guiSettings.textColor
                    elide: Text.ElideRight
                    horizontalAlignment: control.valueType === QEnums.VALUE_TYPE_INT ? Text.AlignRight : Text.AlignLeft
                }
            }

            Component {
                PostHeaderWithAvatar {
                    author: control.value
                    postIndexedSecondsAgo: -1
                    labelsToShow: []
                }
            }
        }
    }

    Component.onDestruction: {
        model.destroy()
    }
}
