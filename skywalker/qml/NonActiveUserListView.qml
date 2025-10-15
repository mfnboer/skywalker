import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ListView {
    required model
    required property string postUri
    required property int action // QEnums::NonActiveUserAction
    property int rowPadding: 2

    signal userClicked(NonActiveUser user)

    id: searchList
    spacing: 0
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned

    Accessible.role: Accessible.List

    delegate: Rectangle {
        required property NonActiveUser modelData
        property alias user: userEntry.modelData

        id: userEntry
        width: searchList.width
        height: grid.height
        color: guiSettings.backgroundColor

        Accessible.role: Accessible.Button
        Accessible.name: user.profile.name
        Accessible.onPressAction: userClicked(user)

        GridLayout {
            id: grid
            width: parent.width
            columns: 3
            rowSpacing: 0
            columnSpacing: 10

            // Avatar
            Rectangle {
                id: avatar
                Layout.rowSpan: 2
                Layout.preferredWidth: 44
                Layout.preferredHeight: avatarImg.height
                Layout.fillHeight: true
                color: "transparent"

                Accessible.ignored: true

                Avatar {
                    id: avatarImg
                    anchors.centerIn: parent
                    width: parent.width - 12
                    author: userEntry.user.profile
                    onClicked: userClicked(userEntry.user)
                }
            }

            AuthorNameAndStatus {
                Layout.topMargin: rowPadding
                Layout.fillWidth: true
                author: user.profile
            }

            NonActiveUserActionIcon {
                id: expiredIcon
                svg: SvgOutline.warning
                iconColor: guiSettings.disabledColor
                visible: user.sessionExpired
            }

            NonActiveUserActionIcon {
                id: notFoundIcon
                svg: SvgOutline.block
                iconColor: guiSettings.disabledColor
                visible: !user.sessionExpired && Boolean(user.postView) &&
                         user.postView.uri === postUri && user.postView.notFound
            }

            NonActiveUserActionIcon {
                id: errorIcon
                svg: SvgOutline.error
                iconColor: guiSettings.errorColor
                visible: !user.sessionExpired && Boolean(user.postView) &&
                         user.postView.uri === postUri && user.postView.hasError()
            }

            NonActiveUserActionIcon {
                id: likeIcon
                svg: user.postView?.likeUri ? SvgFilled.like : SvgOutline.like
                iconColor: guiSettings.likeColor
                visible: action == QEnums.NON_ACTIVE_USER_LIKE && !user.sessionExpired &&
                         Boolean(user.postView) && user.postView.uri === postUri &&
                         user.postView.isGood()

                BlinkingOpacity {
                    target: likeIcon
                    running: user.actionInProgress
                }
            }

            NonActiveUserActionIcon {
                id: bookmarkIcon
                svg: user.postView?.bookmarked ? SvgFilled.bookmark : SvgOutline.bookmark
                iconColor: guiSettings.buttonColor
                visible: action == QEnums.NON_ACTIVE_USER_BOOKMARK && !user.sessionExpired &&
                         Boolean(user.postView) && user.postView.uri === postUri &&
                         user.postView.isGood()

                BlinkingOpacity {
                    target: bookmarkIcon
                    running: user.actionInProgress
                }
            }

            BusyIndicator {
                id: progressIcon
                Layout.preferredWidth: 44
                Layout.preferredHeight: Layout.preferredWidth
                Layout.rowSpan: 2
                running: user.getPostInProgress
                visible: running
            }

            Rectangle {
                Layout.rowSpan: 2
                Layout.preferredHeight: 44
                color: "transparent"
                visible: !expiredIcon.visible && !notFoundIcon.visible && !errorIcon.visible &&
                         !likeIcon.visible && !bookmarkIcon.visible && !progressIcon.visible
            }

            Text {
                bottomPadding: rowPadding
                Layout.fillWidth: true
                Layout.fillHeight: true
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: user.profile.handle ? `@${user.profile.handle}` : ""

                Accessible.ignored: true
            }

            Item{}

            Rectangle {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.preferredHeight: contentLabels.height + 5
                color: "transparent"

                ContentLabels {
                    id: contentLabels
                    anchors.left: parent.left
                    anchors.right: undefined
                    contentLabels: user.profile.labels
                    contentAuthorDid: user.profile.did
                }
            }

            Rectangle {
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
            }
        }
        MouseArea {
            z: -1
            anchors.fill: parent
            onClicked: userClicked(user)
        }

        Component.onCompleted: {
            if (!user.sessionExpired && (!Boolean(user.postView) || user.postView.uri !== postUri ||
                                         !user.postView.isGood()))
            {
                user.getPost(postUri)
            }
        }
    }
}
