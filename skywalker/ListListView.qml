import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property string title
    required property var skywalker
    required property int modelId
    property string description

    signal closed

    id: view
    spacing: 0
    model: skywalker.getListListModel(modelId)
    flickDeceleration: guiSettings.flickDeceleration
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    header: SimpleDescriptionHeader {
        title: view.title
        description: view.description
        onClosed: view.closed()

        SvgButton {
            anchors.right: parent.right
            anchors.top: parent.top
            svg: svgOutline.add
            onClicked: newList()
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: ListViewDelegate {
        required property int index

        viewWidth: view.width
        onUpdateList: (list) => editList(list, index)
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
        page.onListUpdated.connect((name, description , avatar) => {
            statusPopup.show(qsTr("List updated."), QEnums.STATUS_LEVEL_INFO, 2)
            view.model.updateEntry(index, name, description, avatar)
            root.popStack()
        })
        page.onClosed.connect(() => { root.popStack() })
        root.pushStack(page)
    }

    Component.onDestruction: {
        skywalker.removeListListModel(modelId)
    }
}
