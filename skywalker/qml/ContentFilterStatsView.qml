import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property string userDid
    required property ContentFilterStatsModel model
    property Skywalker skywalker: root.getSkywalker(userDid)

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
            implicitWidth: column == 1 ? 60 : treeView.width - 60
            //implicitHeight: Math.max(indicator ? indicator.height : 0, contentItem.height) * 1.25

            contentItem: Loader {
                id: valueLoader

                active: true
                sourceComponent: getValueComponent()

                function getValueComponent() {
                    switch (control.valueType) {
                    case QEnums.VALUE_TYPE_BASIC_PROFILE:
                        return authorComp
                    case QEnums.VALUE_TYPE_MUTED_WORD_ENTRY:
                        return mutedWordComp
                    case QEnums.VALUE_TYPE_LABELER_DID:
                        return labelerComp
                    default:
                        return labelComp
                    }
                }
            }

            Component {
                id: labelComp

                Label {
                    rightPadding: 10
                    clip: false
                    text: typeof control.value != 'object' ? control.value : ""
                    color: guiSettings.textColor
                    elide: Text.ElideRight
                    horizontalAlignment: control.valueType === QEnums.VALUE_TYPE_INT ? Text.AlignRight : Text.AlignLeft
                }
            }

            Component {
                id: authorComp

                Rectangle {
                    property basicprofile nullLabeler

                    color: "transparent"
                    implicitHeight: authorValue.implicitHeight

                    PostHeaderWithAvatar {
                        id: authorValue
                        anchors.left: parent.left
                        anchors.right: parent.right
                        userDid: page.userDid
                        author: page.getTypeName(control.value) === 'BasicProfile' ? control.value : parent.nullLabeler
                        postIndexedSecondsAgo: -1
                        labelsToShow: []
                    }
                }
            }

            Component {
                id: mutedWordComp

                Rectangle {
                    property mutedwordentry nullMutedWordEntry

                    color: "transparent"
                    implicitHeight: mutedWord.implicitHeight + 20

                    MutedWordEntry {
                        id: mutedWord
                        y: 10
                        anchors.left: parent.left
                        anchors.right: parent.right
                        horizontalPadding: 0
                        entry: page.getTypeName(control.value) === 'MutedWordEntry' ? control.value : parent.nullMutedWordEntry
                    }
                }
            }

            Component {
                id: labelerComp

                Rectangle {
                    property basicprofile labeler
                    property string labelerDid: typeof control.value == 'string' ? control.value : ""

                    id: labelerField
                    color: "transparent"
                    implicitHeight: authorValue.implicitHeight

                    onLabelerDidChanged: {
                        if (linkUtils.isValidDid(labelerDid))
                            profileUtils.getBasicProfile(labelerDid)
                    }

                    PostHeaderWithAvatar {
                        id: authorValue
                        anchors.left: parent.left
                        anchors.right: parent.right
                        userDid: page.userDid
                        author: labelerField.labeler
                        postIndexedSecondsAgo: -1
                        labelsToShow: []
                    }

                    ProfileUtils {
                        id: profileUtils
                        skywalker: page.skywalker

                        onBasicProfileOk: (profile) => labelerField.labeler = profile
                    }
                }
            }
        }
    }

    LinkUtils {
        id: linkUtils
        skywalker: page.skywalker
    }

    function getTypeName(value) {
        if (typeof value == 'object')
            return value.typeName

        return typeof value
    }

    Component.onDestruction: {
        model.destroy()
    }
}
