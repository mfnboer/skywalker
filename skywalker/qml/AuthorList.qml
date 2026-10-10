import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property int modelId: -1
    required property var getAuthorListFunc
    required property var getAuthorListNextPageFunc
    property bool showFollow: true
    property bool showDescription: true
    property bool showshowVerificationDate: false

    id: authorList
    width: parent.width
    height: parent.height
    clip: true
    spacing: 0
    model: modelId >= 0 ? skywalker.getAuthorListModel(modelId) : null
    preloadNextPageFunc: () => getAuthorListNextPageFunc(modelId)

    delegate: AuthorViewDelegate {
        width: authorFeedView.width
        userDid: authorList.userDid
        showFollow: authorList.showFollow
        showDescription: authorList.showDescription
        showVerificationDate: authorList.showshowVerificationDate
    }

    FlickableRefresher {
        inProgress: Boolean(authorList.model?.getFeedInProgress)
        bottomOvershootFun: () => getAuthorListNextPageFunc(modelId)
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: Boolean(authorList.model?.getFeedInProgress)
    }

    EmptyListIndication {
        svg: SvgOutline.noUsers
        text: qsTr("None")
        list: authorList
    }

    function refresh() {
        getAuthorListFunc(modelId)
    }

    function clear() {
        model.clear()
    }
}
