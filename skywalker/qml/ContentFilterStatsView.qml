import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property string userDid
    required property ContentFilterStatsModel model
    property Skywalker skywalker: root.getSkywalker(userDid)
    readonly property string sideBarTitle: qsTr("Filtered posts")
    readonly property SvgImage sideBarSvg: SvgOutline.hideVisibility

    id: page

    signal closed

    header: SimpleHeader {
        text: sideBarTitle
        headerVisible: !root.showSideBar
        onBack: page.closed()
    }

    footer: DeadFooterMargin {
    }

    HorizontalHeaderView {
        id: treeHeader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        syncView: treeView
        textRole: "value"
        clip: true

        delegate: Label {
            required property int column

            leftPadding: 10
            rightPadding: 10
            text: model[treeHeader.textRole]
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: column == 1 ? Text.AlignRight : Text.AlignLeft
            font.bold: true
            color: guiSettings.textColor
        }
    }

    TreeView {
        id: treeView
        anchors.top: treeHeader.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        clip: true
        model: page.model

        delegate: TreeViewDelegate {
            required property int column
            required property int valueType // QEnums::ValueType
            required property var value
            required property var keyList // list of QString, BasicProfile, MutedWordEntry
            required property int hideReason // QEnums::HideReason

            id: control
            implicitWidth: column == 1 ? 60 : treeView.width - 60
            implicitHeight: 50
            leftMargin: row == 0 ? -20 : 4

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
                    text: typeof control.value != 'object' ? control.value : ""
                    color: control.column == 0 ? guiSettings.textColor : guiSettings.linkColor
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    horizontalAlignment: control.valueType === QEnums.VALUE_TYPE_INT ? Text.AlignRight : Text.AlignLeft

                    MouseArea {
                        anchors.fill: parent
                        enabled: control.column == 1
                        onClicked: page.viewFilteredPosts(control.hideReason, control.keyList)
                    }
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
                    implicitHeight: mutedWord.implicitHeight

                    MutedWordEntry {
                        id: mutedWord
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

    function viewFilteredPosts(hideReason, keyList) {
        console.debug("View filtered posts:", hideReason, keyList)
        const modelId = skywalker.createFilteredPostFeedModel(hideReason, guiSettings.hideReasonLabelColor)
        const postFeedModel = skywalker.getPostFeedModel(modelId)
        page.model.setFilteredPostFeed(postFeedModel, keyList)
        let component = guiSettings.createComponent("FilteredPostFeedView.qml")
        let view = component.createObject(root, { userDid: page.userDid, modelId: modelId })
        view.onClosed.connect(() => { root.popStack() })
        root.pushStack(view)
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
