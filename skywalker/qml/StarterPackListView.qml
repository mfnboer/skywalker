import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    property Skywalker skywalker: root.getSkywalker()
    required property int modelId
    property string description

    signal closed

    id: view
    spacing: 0
    model: skywalker.getStarterPackListModel(modelId)
    clip: true

    Accessible.role: Accessible.List

    header: Rectangle {
        width: parent.width
        height: headerRow.height
        z: guiSettings.headerZLevel
        color: guiSettings.backgroundColor

        RowLayout {
            id: headerRow
            width: parent.width

            AccessibleText {
                Layout.fillWidth: true
                padding: 10
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: view.description
            }

            SvgPlainButton {
                id: addButton
                svg: SvgOutline.add
                accessibleName: qsTr("create new starter pack")
                onClicked: root.newStarterPack(view.model)
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: StarterPackViewDelegate {
        required property int index

        width: view.width
        userDid: skywalker.getUserDid()

        onUpdateStarterPack: (starterPack) => view.editStarterPack(starterPack, index)
        onDeleteStarterPack: (starterPack) => view.deleteStarterPack(starterPack, index)
    }

    FlickableRefresher {
        inProgress: view.model?.getFeedInProgress
        topOvershootFun: () => refresh()
        bottomOvershootFun: () => skywalker.getAuthorStarterPackListNextPage(skywalker.getUserDid(), modelId)
        topText: qsTr("Refresh starter packs")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noLists
        text: qsTr("No starter packs")
        list: view
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: view.model?.getFeedInProgress
    }

    GraphUtils {
        id: graphUtils
        skywalker: view.skywalker // qmllint disable missing-type
    }

    function editStarterPack(starterPack, index) {
        let component = guiSettings.createComponent("EditStarterPack.qml")
        let page = component.createObject(view, {
                skywalker: skywalker,
                starterPack: starterPack
            })
        page.onStarterPackUpdated.connect((cid, name, description, embeddedLinks) => {
            skywalker.showStatusMessage(qsTr("Starter pack updated."), QEnums.STATUS_LEVEL_INFO, 2)
            view.model.getEntry(index)
            view.model.updateEntry(index, cid, name, description, embeddedLinks)
            root.popStack()
        })
        page.onClosed.connect(() => { root.popStack() })
        root.pushStack(page)
    }

    function deleteStarterPack(starterPack, index) {
        guiSettings.askYesNoQuestion(
                    view,
                    qsTr(`Do you really want to delete: ${(starterPack.name)} ?`),
                    () => view.continueDeleteStarterPack(starterPack, index))
    }

    function continueDeleteStarterPack(starterPack, index) {
        view.model.deleteEntry(index)
        graphUtils.deleteStarterPack(starterPack.uri)
    }

    function refresh() {
        skywalker.getAuthorStarterPackList(skywalker.getUserDid(), modelId)
    }

    Component.onDestruction: {
        skywalker.removeStarterPackListModel(modelId)
    }
}
