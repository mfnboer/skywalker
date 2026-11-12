import QtQuick
import skywalker

LinkCardView {
    property string userDid
    required property basicprofile author
    required property actorstatusview authorStatus
    property Skywalker skywalker: root.getSkywalker(userDid)
    property bool canEdit: false

    signal close
    signal edit
    signal report

    id: view
    uri: authorStatus.externalView.uri
    title: authorStatus.externalView.title
    description: authorStatus.externalView.description
    thumbUrl: authorStatus.externalView.thumbUrl
    contentVisibility: !authorStatus.isNull() ? skywalker.getContentVisibility(authorStatus.labels, author) : QEnums.CONTENT_VISIBILITY_SHOW
    contentWarning: !authorStatus.isNull() ? skywalker.getContentWarning(authorStatus.labels, author) : ""
    isLiveExternal: true
    liveExpiresAt: authorStatus.expiresAt

    LiveLabel {
        x: 5
        y: 10
    }

    SvgButton {
        id: closeButton
        anchors.right: parent.right
        width: 34
        height: width
        svg: SvgOutline.close
        visible: canEdit
        onClicked: view.close()
    }

    SvgButton {
        anchors.right: closeButton.left
        width: 34
        height: width
        svg: SvgOutline.edit
        visible: canEdit
        onClicked: view.edit()
    }

    AccessibleText {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        padding: 10
        color: guiSettings.linkColor
        text: qsTr("Report")
        visible: !canEdit && authorStatus.uri && authorStatus.cid

        MouseArea {
            anchors.fill: parent
            onClicked: view.report()
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: view.skywalker
        onBasicProfileOk: (profile) => contentLabeler = profile
    }

    Component.onCompleted: {
        if (!authorStatus.isNull()) {
            const labelerDid = skywalker.getContentLabelerDid(authorStatus.labels, author)

            if (labelerDid)
                profileUtils.getBasicProfile(labelerDid)
        }
    }
}
