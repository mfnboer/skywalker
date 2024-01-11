import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    required property int modelId
    property string description
    property bool ownLists: true

    signal closed

    id: view
    spacing: 0
    model: skywalker.getListListModel(modelId)
    flickDeceleration: guiSettings.flickDeceleration
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: headerRow.height
        z: guiSettings.headerZLevel
        color: guiSettings.backgroundColor

        RowLayout {
            id: headerRow
            width: parent.width

            Text {
                Layout.fillWidth: true
                padding: 10
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: view.description
            }

            SvgButton {
                id: addButton
                svg: svgOutline.add
                visible: ownLists
                onClicked: newList()
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: ListViewDelegate {
        required property int index

        viewWidth: view.width
        ownLists: view.ownLists

        onUpdateList: (list) => view.editList(list, index)
        onDeleteList: (list) => view.deleteList(list, index)
        onListClicked: (list) => root.viewList(list)
    }

    FlickableRefresher {
        inProgress: skywalker.getListListInProgress
        verticalOvershoot: view.verticalOvershoot
        topOvershootFun: () => skywalker.getListList(modelId)
        bottomOvershootFun: () => skywalker.getListListNextPage(modelId)
        topText: qsTr("Refresh lists")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.noLists
        text: qsTr("No lists")
        list: view
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: skywalker.getListListInProgress
    }

    GraphUtils {
        id: graphUtils
        skywalker: view.skywalker

        onDeleteListFailed: (error) => {
            statusPopup.show(qsTr(`Failed to delete list: ${error}`), QEnums.STATUS_LEVEL_ERROR)
            skywalker.getListList(modelId)
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function newList() {
        let component = Qt.createComponent("EditList.qml")
        let page = component.createObject(view, {
                skywalker: skywalker,
                purpose: model.getType()
            })
        page.onListCreated.connect((list) => {
            if (list.isNull()) {
                // This should rarely happen. Let the user refresh.
                statusPopup.show(qsTr("List created. Please refresh page."), QEnums.STATUS_LEVEL_INFO);
            }
            else {
                statusPopup.show(qsTr("List created."), QEnums.STATUS_LEVEL_INFO, 2)
                view.model.prependList(list)
            }

            root.popStack()
        })
        page.onClosed.connect(() => { root.popStack() })
        root.pushStack(page)
    }

    function editList(list, index) {
        let component = Qt.createComponent("EditList.qml")
        let page = component.createObject(view, {
                skywalker: skywalker,
                purpose: list.purpose,
                list: list
            })
        page.onListUpdated.connect((cid, name, description , avatar) => {
            statusPopup.show(qsTr("List updated."), QEnums.STATUS_LEVEL_INFO, 2)
            view.model.updateEntry(index, cid, name, description, avatar)
            root.popStack()
        })
        page.onClosed.connect(() => { root.popStack() })
        root.pushStack(page)
    }

    function deleteList(list, index) {
        guiSettings.askYesNoQuestion(
                    view,
                    qsTr(`Do you really want to delete: ${(list.name)} ?`),
                    () => view.continueDeleteList(list, index))
    }

    function continueDeleteList(list, index) {
        view.model.deleteEntry(index)
        graphUtils.deleteList(list.uri)
    }

    Component.onDestruction: {
        skywalker.removeListListModel(modelId)
    }
}
